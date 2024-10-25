#include <sqlite3.h>

#include "component.hpp"
#include "sql_utility.hpp"
#include "static_linked_list.hpp"

namespace ecsql {

RawComponent::RawComponent(std::string_view name, const std::vector<std::string>& fields, const std::string& additional_schema, bool allow_duplicate)
	: name(name)
	, fields(fields)
	, allow_duplicate(allow_duplicate)
	, additional_schema(additional_schema)
{
	STATIC_LINKED_LIST_INSERT();
}

RawComponent::RawComponent(std::string_view name, std::vector<std::string>&& fields, const std::string& additional_schema, bool allow_duplicate)
	: name(name)
	, fields(fields)
	, allow_duplicate(allow_duplicate)
	, additional_schema(additional_schema)
{
	STATIC_LINKED_LIST_INSERT();
}

void RawComponent::prepare(sqlite3 *db) {
	execute_sql_script(db, schema_sql().c_str());
}

int RawComponent::entity_id_index() const {
	return allow_duplicate ? 1 : 0;
}

int RawComponent::first_field_index() const {
	return allow_duplicate ? 2 : 1;
}

std::string RawComponent::schema_sql() const {
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

std::string RawComponent::insert_sql() const {
	std::string query;
	query = "INSERT INTO ";
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

std::string RawComponent::update_sql() const {
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

const std::string& RawComponent::get_name() const {
	return name;
}

const std::vector<std::string>& RawComponent::get_fields() const {
	return fields;
}

}
