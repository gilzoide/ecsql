#pragma once

#include <string_view>

#include <sqlite3.h>

namespace ecsql {

struct Entity {
    Entity();
    Entity(sqlite3_int64 id);

    sqlite3_int64 id;

	inline static std::string_view schema_sql = (
		"CREATE TABLE entity(id INTEGER PRIMARY KEY, name TEXT);\n"
		"CREATE INDEX entity_name ON entity(name);"
	);
	inline static std::string_view insert_sql = "INSERT INTO entity(name) VALUES(?) RETURNING id";
	inline static std::string_view delete_sql = "DELETE FROM entity WHERE id = ?";
};
	
}

