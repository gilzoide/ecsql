#include <cstdio>

#include "background_system.hpp"
#include "component.hpp"
#include "hook_system.hpp"
#include "prepared_sql.hpp"
#include "screen.hpp"
#include "sql_hook_row.hpp"
#include "sql_utility.hpp"
#include "system.hpp"
#include "time.hpp"
#include "world.hpp"
#include "world_schema.h"

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

	execute_sql_script(db, world_schema);

	return db;
}

World::World()
	: World(DEFAULT_DB_NAME)
{
}

World::World(const char *db_name)
	: db(ecsql_create_db(db_name ?: DEFAULT_DB_NAME), sqlite3_close_v2)
	, begin_stmt(db.get(), "BEGIN", true)
	, commit_stmt(db.get(), "COMMIT", true)
	, rollback_stmt(db.get(), "ROLLBACK", true)
	, create_entity_stmt(db.get(), Entity::insert_sql, true)
	, delete_entity_stmt(db.get(), Entity::delete_sql, true)
	, delete_entity_by_name_stmt(db.get(), Entity::delete_by_name_sql, true)
	, find_entity_stmt(db.get(), Entity::find_by_name_sql, true)
	, update_delta_time_stmt(db.get(), time::update_delta_sql, true)
	, select_fixed_delta_time_stmt(db.get(), time::select_fixed_delta_time_sql, true)
#ifdef __EMSCRIPTEN__
	, dispatch_queue(0)
#else
	, dispatch_queue(2)
#endif
{
#ifdef TRACY_ENABLE
	for (int i = 0; i < dispatch_queue.thread_count(); i++) {
		dispatch_queue.dispatch_forget([=]() {
			tracy::SetThreadName(std::format("ecsql-{}", i).c_str());
		});
	}
#endif
	sqlite3_preupdate_hook(db.get(), preupdate_hook, this);
}

World::~World() {
	if (LAST_DB_NAME[0]) {
		backup_into(LAST_DB_NAME);
	}
	execute_all_prehooks(HookType::OnDelete);
}

void World::register_component(const Component& component) {
	component.prepare(db.get());
}
void World::register_component(Component&& component) {
	component.prepare(db.get());
}

void World::register_system(const System& system, bool use_fixed_delta) {
	std::vector<PreparedSQL> prepared_sql;
	system.prepare(db.get(), prepared_sql);
	(use_fixed_delta ? fixed_systems : systems).emplace_back(system, std::move(prepared_sql));
}

void World::register_system(System&& system, bool use_fixed_delta) {
	std::vector<PreparedSQL> prepared_sql;
	system.prepare(db.get(), prepared_sql);
	(use_fixed_delta ? fixed_systems : systems).emplace_back(std::move(system), std::move(prepared_sql));
}

void World::remove_system(std::string_view system_name) {
	std::erase_if(systems, [system_name](std::tuple<System, std::vector<PreparedSQL>> t) {
		return std::get<0>(t).name == system_name;
	});
}

void World::remove_system(const System& system) {
	remove_system(system.name);
}

void World::remove_systems_with_prefix(std::string_view system_name_prefix) {
	std::erase_if(systems, [system_name_prefix](std::tuple<System, std::vector<PreparedSQL>> t) {
		return std::get<0>(t).name.starts_with(system_name_prefix);
	});
}

void World::register_hook_system(const HookSystem& system) {
	auto it = hook_systems.find(system.component_name);
	if (it == hook_systems.end()) {
		it = hook_systems.emplace(system.component_name, std::vector<HookSystem>{}).first;
	}
	it->second.push_back(system);
}

void World::register_hook_system(HookSystem&& system) {
	auto it = hook_systems.find(system.component_name);
	if (it == hook_systems.end()) {
		it = hook_systems.emplace(system.component_name, std::vector<HookSystem>{}).first;
	}
	it->second.push_back(std::move(system));
}

void World::register_background_system(const BackgroundSystem& system) {
	background_systems.emplace_back(system, std::future<void>());
}

void World::register_background_system(BackgroundSystem&& system) {
	background_systems.emplace_back(std::move(system), std::future<void>());
}

void World::remove_background_system(std::string_view system_name) {
	std::erase_if(background_systems, [system_name](std::pair<BackgroundSystem, std::future<void>>& t) {
		return t.first.get_name() == system_name;
	});
}

void World::remove_background_system(const BackgroundSystem& system) {
	remove_background_system(system.get_name());
}

void World::remove_background_systems_with_prefix(std::string_view system_name_prefix) {
	std::erase_if(background_systems, [system_name_prefix](std::pair<BackgroundSystem, std::future<void>>& t) {
		return t.first.get_name().starts_with(system_name_prefix);
	});
}

EntityID World::create_entity(std::optional<std::string_view> name, std::optional<EntityID> parent) {
	create_entity_stmt(name, parent);
	return sqlite3_last_insert_rowid(db.get());
}

int World::delete_entity(EntityID id) {
	delete_entity_stmt(id);
	return sqlite3_changes(db.get());
}

int World::delete_entity(std::string_view name) {
	delete_entity_by_name_stmt(name);
	return sqlite3_changes(db.get());
}

std::optional<EntityID> World::find_entity(std::string_view name) {
	if (auto it = find_entity_stmt(name).begin()) {
		return it.row().get<EntityID>();
	}
	else {
		return std::nullopt;
	}
}


void World::begin_transaction() {
	ZoneScopedN("begin_transaction");
	if (commit_or_rollback_result.valid()) {
		commit_or_rollback_result.get();
	}
	begin_stmt();
}

void World::commit_transaction() {
	ZoneScoped;
	if (commit_or_rollback_result.valid()) {
		commit_or_rollback_result.get();
	}
	commit_or_rollback_result = dispatch_queue.dispatch([this]() {
		ZoneScopedN("commit_transaction.async");
		commit_stmt();
	});
}

void World::rollback_transaction() {
	ZoneScoped;
	if (commit_or_rollback_result.valid()) {
		commit_or_rollback_result.get();
	}
	commit_or_rollback_result = dispatch_queue.dispatch([this]() {
		ZoneScoped;
		commit_stmt();
	});
}

void World::update(float delta_time) {
	inside_transaction([=](World& self) {
		{
			ZoneScopedN("update_delta_time");
			self.update_delta_time_stmt(delta_time);
		}

		// fixed update
		float fixed_delta_time = self.select_fixed_delta_time_stmt().get<float>();
		self.fixed_delta_executor.execute(delta_time, fixed_delta_time, [&]() {
			for (auto&& [system, prepared_sql] : self.fixed_systems) {
				system(self, prepared_sql);
			}
		});

		// regular update
		for (auto&& [system, prepared_sql] : self.systems) {
			system(self, prepared_sql);
		}

		// lastly, dispatch background systems
		for (auto&& [system, future] : self.background_systems) {
			if (future.valid()) {
				future.get();
			}
			future = self.dispatch_queue.dispatch([&]() { system(); });
		}
	});
}

void World::on_window_resized(int new_width, int new_height) {
	execute_sql(screen::update_sql, new_width, new_height);
}

bool World::backup_into(const char *db_name) {
	sqlite3 *db;
	if (sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
		std::cerr << "Error backing up into \"" << db_name << "\": " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	bool result = backup_into(db);
	sqlite3_close(db);
	return result;
}

bool World::backup_into(sqlite3 *db) {
	sqlite3_backup *backup = sqlite3_backup_init(db, "main", this->db.get(), "main");
	sqlite3_backup_step(backup, -1);
	return sqlite3_backup_finish(backup) == SQLITE_OK;
}

bool World::restore_from(const char *db_name) {
	sqlite3 *db;
	if (sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
		std::cerr << "Error restoring from \"" << db_name << "\": " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	bool result = restore_from(db);
	sqlite3_close(db);
	return result;
}

bool World::restore_from(sqlite3 *db) {
	execute_all_prehooks(HookType::OnDelete);
	sqlite3_backup *backup = sqlite3_backup_init(this->db.get(), "main", db, "main");
	sqlite3_backup_step(backup, -1);
	execute_all_prehooks(HookType::OnInsert);
	return sqlite3_backup_finish(backup) == SQLITE_OK;
}

std::shared_ptr<sqlite3> World::get_db() const {
	return db;
}

PreparedSQL World::prepare_sql(std::string_view sql, bool is_persistent) {
	return PreparedSQL(db.get(), sql, is_persistent);
}

void World::execute_sql_script(const char *sql) {
	ecsql::execute_sql_script(db.get(), sql);
}

void World::create_function(const char *name, int argument_count, void (*fn)(sqlite3_context*, int, sqlite3_value**)) {
	sqlite3_create_function(db.get(), name, argument_count, SQLITE_UTF8, nullptr, fn, nullptr, nullptr);
}

// private methods
void World::preupdate_hook(
	void *pCtx,                   /* Copy of third arg to preupdate_hook() */
	sqlite3 *db,                  /* Database handle */
	int op,                       /* SQLITE_UPDATE, DELETE or INSERT */
	char const *zDb,              /* Database name */
	char const *zName,            /* Table name */
	sqlite3_int64 iKey1,          /* Rowid of row about to be deleted/updated */
	sqlite3_int64 iKey2           /* New rowid value (for a rowid UPDATE) */
) {
	World *world = (World *) pCtx;
	switch (op) {
		case SQLITE_INSERT:
			world->execute_prehook(zName, HookType::OnInsert, iKey1, iKey2);
			break;

		case SQLITE_DELETE:
			world->execute_prehook(zName, HookType::OnDelete, iKey1, iKey2);
			break;

		case SQLITE_UPDATE:
			world->execute_prehook(zName, HookType::OnUpdate, iKey1, iKey2);
			break;
	}
}

void World::execute_prehook(const char *table, HookType hook, sqlite3_int64 old_rowid, sqlite3_int64 new_rowid) {
	auto it = hook_systems.find(table);
	if (it != hook_systems.end()) {
		SQLHookRow old_row { db.get(), old_rowid, false };
		SQLHookRow new_row { db.get(), new_rowid, true };
		for (auto& system : it->second) {
			system(hook, old_row, new_row);
		}
	}
}

void World::execute_all_prehooks(HookType hook) {
	for (auto it : hook_systems) {
		std::string sql = "SELECT * FROM ";
		sql += it.first;
		PreparedSQL select_all(db.get(), sql, false);
		for (SQLRow row : select_all()) {
			for (auto& system : it.second) {
				system(hook, row, row);
			}
		}
	}
}

}
