#include "component.hpp"

namespace ecsql {

Component::Component(const std::string& name, const std::vector<std::string>& fields)
	: name(name)
	, fields(fields)
{
}
Component::Component(const std::string& name, std::vector<std::string>&& fields)
	: name(name)
	, fields(fields)
{
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
	query += "SET ";
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
