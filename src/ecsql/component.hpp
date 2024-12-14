#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <reflect>

#include "entity.hpp"
#include "prepared_sql.hpp"
#include "static_linked_list.hpp"

typedef struct sqlite3 sqlite3;

namespace ecsql {

class Component {
public:
	Component(std::string_view name, const std::vector<std::string>& fields, const std::string& additional_schema = "", bool allow_duplicate = false);
	Component(std::string_view name, std::vector<std::string>&& fields, const std::string& additional_schema = "", bool allow_duplicate = false);

	void prepare(sqlite3 *db);

	// If `allow_duplicate`, entity_id is in index 1. Otherwise it's in index 0.
	int entity_id_index() const;
	// If `allow_duplicate`, first field index is 2. Otherwise it's in index 1.
	int first_field_index() const;

	std::string schema_sql() const;
	std::string insert_sql(bool or_replace = false) const;
	std::string update_sql() const;

	const std::string& get_name() const;
	const std::vector<std::string>& get_fields() const;

protected:
	std::string name;
	std::vector<std::string> fields;
	std::string additional_schema;
	bool allow_duplicate;

	STATIC_LINKED_LIST_DEFINE(Component);
};

class Tag : public Component {
public:
	Tag(std::string_view name) : Component(name, {}) {}
};

}
