#pragma once

#include <string_view>

#include <sqlite3.h>

namespace ecsql {

typedef sqlite3_int64 EntityID;

struct Entity {
    EntityID id;
	std::string_view name;
    EntityID parent_id;

	inline static const char insert_sql[] = "INSERT INTO entity(name, parent_id) VALUES(?, ?) RETURNING id";
	inline static const char delete_sql[] = "DELETE FROM entity WHERE id = ?";
};
	
}

