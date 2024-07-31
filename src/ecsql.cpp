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
	, create_entity_stmt(db, "INSERT INTO entity(id) VALUES(NULL)", true)
	, delete_entity_stmt(db, "DELETE FROM entity WHERE id = ?", true)
{
}

Ecsql::~Ecsql() {
	sqlite3_close(db);
	db = nullptr;
}

void Ecsql::register_component(const Component& component) {
	std::string query;
	query = "CREATE TABLE ";
	query += component.name;
	query += "(\n  id INTEGER PRIMARY KEY,\n  entity_id INTEGER NOT NULL REFERENCES entity(id) ON DELETE CASCADE";
	for (auto& it : component.fields) {
		query += ",\n  ";
		query += it;
	}
	query += "\n);\nCREATE INDEX ";
	query += component.name;
	query += "_entity_id ON ";
	query += component.name;
	query += "(entity_id);";
	int res = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
}

std::function<void()> Ecsql::register_system(const std::string& query, std::function<void(const SQLRow&)> f) {
	sqlite3 *db = this->db;
	return [=]() {
		PreparedSQL sql(db, query, true);
		while (true) {
			int res = sql.step();
			switch (res) {
				case SQLITE_ROW: {
					SQLRow row(sql);
					f(row);
					break;
				}
				
				case SQLITE_DONE:
					return;
				
				default:
					std::cout << sqlite3_errmsg(db) << std::endl;
					return;
			}
		}
	};
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
	catch (...) {
		rollback_stmt.reset().step();
	}
}

void Ecsql::inside_transaction(std::function<void(Ecsql&)> f) {
	begin_stmt.reset().step();
	try {
		f(*this);
		commit_stmt.reset().step();
	}
	catch (...) {
		rollback_stmt.reset().step();
	}
}

void Ecsql::inside_transaction(std::function<void(sqlite3 *)> f) {
	begin_stmt.reset().step();
	try {
		f(db);
		commit_stmt.reset().step();
	}
	catch (...) {
		rollback_stmt.reset().step();
	}
}

}
