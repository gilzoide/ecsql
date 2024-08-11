#include <sqlite3.h>

#include "component.hpp"
#include "sql_row.hpp"

namespace ecsql {

Component::Component(std::string_view name, const std::vector<std::string>& fields)
	: name(name)
	, fields(fields)
{
}

Component::Component(std::string_view name, std::vector<std::string>&& fields)
	: name(name)
	, fields(fields)
{
}

void Component::prepare(sqlite3 *db) {
	int res = sqlite3_exec(db, schema_sql().c_str(), nullptr, nullptr, nullptr);
	if (res != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}

	insert_stmt = PreparedSQL(db, insert_sql(), true);
	update_stmt = PreparedSQL(db, update_sql(), true);
}

std::string Component::schema_sql() const {
	std::string query;
	query = "CREATE TABLE ";
	query += name;
	query += "(\n  id INTEGER PRIMARY KEY,\n  entity_id INTEGER NOT NULL REFERENCES entity(id) ON DELETE CASCADE";
	for (auto& it : fields) {
		query += ",\n  ";
		query += it;
	}
	query += "\n);\nCREATE INDEX ";
	query += name;
	query += "_entity_id ON ";
	query += name;
	query += "(entity_id);";
	return query;
}

std::string Component::insert_sql() const {
	std::string query;
	query = "INSERT INTO ";
	query += name;
	query += "(entity_id";
	for (auto& it : fields) {
		query += ", ";
		query += it;
	}
	query += ") VALUES(?";
	for (auto& it : fields) {
		query += ", ?";
	}
	query += ")";
	return query;
}

std::string Component::update_sql() const {
	std::string query;
	query = "UPDATE ";
	query += name;
	query += " SET ";
	bool first_it = true;
	for (auto& it : fields) {
		if (first_it) {
			first_it = false;
		}
		else {
			query += ", ";
		}
		query += it;
		query += " = ?";
	}
	query += "WHERE entity_id = ?";
	return query;
}

}
