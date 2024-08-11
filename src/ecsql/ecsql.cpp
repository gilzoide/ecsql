#include "ecsql.hpp"
#include "PreparedSQL.hpp"
#include "SQLRow.hpp"
#include "component.hpp"

#include <cstdio>
#include <iostream>
#include <functional>
#include <stdexcept>

namespace ecsql {

static const char MEMORY_DB_NAME[] = ":memory:";

sqlite3 *create_db(const char *db_name) {
	std::remove(db_name);

	sqlite3 *db;
	int res = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	res = sqlite3_exec(db, "CREATE TABLE entity(id INTEGER PRIMARY KEY)", nullptr, nullptr, nullptr);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	res = sqlite3_exec(db, "PRAGMA journal_mode = off", nullptr, nullptr, nullptr);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	return db;
}

Ecsql::Ecsql()
	: Ecsql(MEMORY_DB_NAME)
{
}

Ecsql::Ecsql(const char *db_name)
	: Ecsql(create_db(db_name ?: MEMORY_DB_NAME))
{
}

Ecsql::Ecsql(sqlite3 *db)
	: db(db ?: create_db(MEMORY_DB_NAME))
	, begin_stmt(db, "BEGIN", true)
	, commit_stmt(db, "COMMIT", true)
	, rollback_stmt(db, "ROLLBACK", true)
	, create_entity_stmt(db, "INSERT INTO entity DEFAULT VALUES", true)
	, delete_entity_stmt(db, "DELETE FROM entity WHERE id = ?", true)
{
}

Ecsql::~Ecsql() {
	sqlite3_close(db);
	db = nullptr;
}

void Ecsql::register_component(const Component& component) {
	int res = sqlite3_exec(db, component.schema_sql().c_str(), nullptr, nullptr, nullptr);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
}

void Ecsql::register_system(const System& system) {
	System copy = system;
	register_system(std::move(copy));
}

void Ecsql::register_system(System&& system) {
	system.prepare(db);
	systems.push_back(system);
}

entity_id Ecsql::create_entity() {
	int res = create_entity_stmt.reset().step();
	if (res != SQLITE_OK && res != SQLITE_ROW && res != SQLITE_DONE) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	return create_entity_stmt.column_int64(0);
}

bool Ecsql::delete_entity(entity_id id) {
	int res = delete_entity_stmt.reset().bind(1, id).step();
	if (res != SQLITE_OK && res != SQLITE_ROW && res != SQLITE_DONE) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	return delete_entity_stmt.column_bool(0);
}

void Ecsql::inside_transaction(std::function<void()> f) {
	begin_stmt.reset().step();
	try {
		f();
		commit_stmt.reset().step();
	}
	catch (std::runtime_error& err) {
		std::cerr << "Runtime error: " << err.what() << std::endl;
		rollback_stmt.reset().step();
	}
}

void Ecsql::inside_transaction(std::function<void(Ecsql&)> f) {
	begin_stmt.reset().step();
	try {
		f(*this);
		commit_stmt.reset().step();
	}
	catch (std::runtime_error& err) {
		std::cerr << "Runtime error: " << err.what() << std::endl;
		rollback_stmt.reset().step();
	}
}

void Ecsql::update() {
	inside_transaction([](Ecsql& self) {
		for (auto& it : self.systems) {
			it();
		}
	});
}

sqlite3 *Ecsql::get_db() const {
	return db;
}

}
