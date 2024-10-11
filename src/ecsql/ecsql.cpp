#include <cstdio>
#include <fstream>

#include <toml++/toml.hpp>

#include "component.hpp"
#include "ecsql.hpp"
#include "hook_system.hpp"
#include "sql_hook_row.hpp"
#include "system.hpp"

namespace ecsql {

#if defined(DEBUG) && !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
static const char DEFAULT_DB_NAME[] = "ecsql_world.sqlite3";
static const char LAST_DB_NAME[] = "ecsql_world-backup10.sqlite3";
#else
static const char DEFAULT_DB_NAME[] = ":memory:";
static const char LAST_DB_NAME[] = "";
#endif

static sqlite3 *ecsql_create_db(const char *db_name) {
	std::remove(db_name);

	sqlite3 *db;
	int res = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	res = sqlite3_exec(db, Entity::schema_sql(), nullptr, nullptr, nullptr);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	return db;
}

static void ecsql_preupdate_hook(
    void *pCtx,                   /* Copy of third arg to preupdate_hook() */
    sqlite3 *db,                  /* Database handle */
    int op,                       /* SQLITE_UPDATE, DELETE or INSERT */
    char const *zDb,              /* Database name */
    char const *zName,            /* Table name */
    sqlite3_int64 iKey1,          /* Rowid of row about to be deleted/updated */
    sqlite3_int64 iKey2           /* New rowid value (for a rowid UPDATE) */
) {
	Ecsql *world = (Ecsql *) pCtx;
	switch (op) {
		case SQLITE_INSERT:
			world->on_insert(zName);
			break;

		case SQLITE_DELETE:
			world->on_delete(zName);
			break;

		case SQLITE_UPDATE:
			world->on_update(zName);
			break;
	}
}

Ecsql::Ecsql()
	: Ecsql(DEFAULT_DB_NAME)
{
}

Ecsql::Ecsql(const char *db_name)
	: Ecsql(ecsql_create_db(db_name ?: DEFAULT_DB_NAME))
{
}

Ecsql::Ecsql(sqlite3 *db)
	: db(db ?: ecsql_create_db(DEFAULT_DB_NAME))
	, begin_stmt(db, "BEGIN", true)
	, commit_stmt(db, "COMMIT", true)
	, rollback_stmt(db, "ROLLBACK", true)
	, create_entity_stmt(db, "INSERT INTO entity(name) VALUES(?) RETURNING id", true)
	, delete_entity_stmt(db, "DELETE FROM entity WHERE id = ?", true)
{
	execute_sql("PRAGMA foreign_keys = 1");
	sqlite3_preupdate_hook(db, ecsql_preupdate_hook, this);
}

Ecsql::~Ecsql() {
	sqlite3_close_v2(db);
	if (LAST_DB_NAME[0]) {
		backup_into(LAST_DB_NAME);
	}
}

void Ecsql::register_component(RawComponent& component) {
	component.prepare(db);
}
void Ecsql::register_component(RawComponent&& component) {
	component.prepare(db);
}

void Ecsql::register_system(System& system) {
	system.prepare(db);
	systems.push_back(system);
}

void Ecsql::register_system(System&& system) {
	system.prepare(db);
	systems.push_back(system);
}

void Ecsql::register_hook_system(const HookSystem& system) {
	switch (system.hook_type) {
		case HookType::OnInsert:
			register_prehook(on_insert_systems, system);
			break;
		case HookType::OnUpdate:
			register_prehook(on_update_systems, system);
			break;
		case HookType::OnDelete:
			register_prehook(on_delete_systems, system);
			break;
	}
}
void Ecsql::register_hook_system(HookSystem&& system) {
	switch (system.hook_type) {
		case HookType::OnInsert:
			register_prehook(on_insert_systems, system);
			break;
		case HookType::OnUpdate:
			register_prehook(on_update_systems, system);
			break;
		case HookType::OnDelete:
			register_prehook(on_delete_systems, system);
			break;
	}
}

Entity Ecsql::create_entity() {
	return create_entity_stmt(nullptr).get<Entity>();
}

Entity Ecsql::create_entity(std::string_view name) {
	return create_entity_stmt(name).get<Entity>();
}

bool Ecsql::delete_entity(Entity id) {
	return delete_entity_stmt(id).get<bool>();
}

void Ecsql::update() {
	inside_transaction([](Ecsql& self) {
		for (auto& system : self.systems) {
			system();
		}
	});
}

void Ecsql::on_insert(const char *table) {
	execute_prehook(table, on_insert_systems);
}

void Ecsql::on_delete(const char *table) {
	execute_prehook(table, on_delete_systems);
}

void Ecsql::on_update(const char *table) {
	execute_prehook(table, on_update_systems);
}

bool Ecsql::backup_into(const char *db_name) {
	sqlite3 *db;
	if (sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
		std::cerr << "Error backing up into \"" << db_name << "\": " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	bool result = backup_into(db);
	sqlite3_close(db);
	return result;
}

bool Ecsql::backup_into(sqlite3 *db) {
	sqlite3_backup *backup = sqlite3_backup_init(db, "main", this->db, "main");
	sqlite3_backup_step(backup, -1);
	return sqlite3_backup_finish(backup) == SQLITE_OK;
}

bool Ecsql::restore_from(const char *db_name) {
	sqlite3 *db;
	if (sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
		std::cerr << "Error restoring from \"" << db_name << "\": " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	bool result = restore_from(db);
	sqlite3_close(db);
	return result;
}

bool Ecsql::restore_from(sqlite3 *db) {
	sqlite3_backup *backup = sqlite3_backup_init(this->db, "main", db, "main");
	sqlite3_backup_step(backup, -1);
	return sqlite3_backup_finish(backup) == SQLITE_OK;
}

sqlite3 *Ecsql::get_db() const {
	return db;
}

// private methods
void Ecsql::register_prehook(std::unordered_map<std::string, std::vector<HookSystem>>& map, const HookSystem& system) {
	auto it = map.find(system.component_name);
	if (it == map.end()) {
		it = map.emplace(system.component_name, std::vector<HookSystem>{}).first;
	}
	it->second.push_back(system);
}
void Ecsql::register_prehook(std::unordered_map<std::string, std::vector<HookSystem>>& map, HookSystem&& system) {
	auto it = map.find(system.component_name);
	if (it == map.end()) {
		it = map.emplace(system.component_name, std::vector<HookSystem>{}).first;
	}
	it->second.push_back(system);
}

void Ecsql::execute_prehook(const char *table, const std::unordered_map<std::string, std::vector<HookSystem>>& map) {
	auto it = map.find(table);
	if (it != map.end()) {
		SQLHookRow old_row { db, false };
		SQLHookRow new_row { db, true };
		for (auto& system : it->second) {
			system(old_row, new_row);
		}
	}
}

}
