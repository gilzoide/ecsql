#pragma once

#include <string_view>

#include <sqlite3.h>

namespace ecsql {

struct Entity {
    Entity();
    Entity(sqlite3_int64 id);

    sqlite3_int64 id;

	inline static const char insert_sql[] = "INSERT INTO entity(name) VALUES(?) RETURNING id";
	inline static const char delete_sql[] = "DELETE FROM entity WHERE id = ?";
};
	
}

