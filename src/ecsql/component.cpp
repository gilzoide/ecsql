#include <sqlite3.h>

#include "component.hpp"
#include "sql_utility.hpp"
#include "static_linked_list.hpp"

namespace ecsql {

Component::Component(std::string_view name, const std::vector<std::string>& fields, const std::string& additional_schema, bool allow_duplicate)
	: name(name)
	, fields(fields)
	, allow_duplicate(allow_duplicate)
	, additional_schema(additional_schema)
{
	STATIC_LINKED_LIST_INSERT();
}

Component::Component(std::string_view name, std::vector<std::string>&& fields, const std::string& additional_schema, bool allow_duplicate)
	: name(name)
	, fields(fields)
	, allow_duplicate(allow_duplicate)
	, additional_schema(additional_schema)
{
	STATIC_LINKED_LIST_INSERT();
}

void Component::prepare(sqlite3 *db) const {
	execute_sql_script(db, schema_sql().c_str());
}

int Component::entity_id_index() const {
	return allow_duplicate ? 1 : 0;
}

int Component::first_field_index() const {
	return allow_duplicate ? 2 : 1;
}

std::string Component::schema_sql() const {
	std::string query;
	query = "CREATE TABLE ";
	query += name;
	if (allow_duplicate) {
		query += "(\n  id INTEGER PRIMARY KEY,\n  entity_id INTEGER NOT NULL REFERENCES entity(id) ON DELETE CASCADE";
	}
	else {
		query += "(\n  entity_id INTEGER PRIMARY KEY REFERENCES entity(id) ON DELETE CASCADE";
	}
	for (auto& it : fields) {
		query += ",\n  ";
		query += it;
	}
	query += "\n);";
	if (allow_duplicate) {
		query += "\nCREATE INDEX ";
		query += name;
		query += "_entity_id ON ";
		query += name;
		query += "(entity_id);";
	}
	if (!additional_schema.empty()) {
		query += "\n";
		query += additional_schema;
	}
	return query;
}

std::string Component::insert_sql(bool or_replace) const {
	std::string query;
	query = "INSERT ";
	if (or_replace) {
		query += "OR REPLACE ";
	}
	query += "INTO ";
	query += name;
	query += "(entity_id";
	for (auto& it : fields) {
		query += ", ";
		query += extract_identifier(it);
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
	for (int i = 0; i < fields.size(); i++) {
		if (i > 0) {
			query += ", ";
		}
		query += extract_identifier(fields[i]);
		query += " = ?";
		query += std::to_string(i + 2);
	}
	query += " WHERE entity_id = ?1";
	return query;
}

const std::string& Component::get_name() const {
	return name;
}

const std::vector<std::string>& Component::get_fields() const {
	return fields;
}

}
