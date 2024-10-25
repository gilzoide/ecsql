#pragma once

#include <string_view>

namespace ecsql {

struct time {
	float delta;

	inline static const char schema_sql[] = "CREATE TABLE time(delta)";
	inline static const char insert_singleton_sql[] = "INSERT INTO time(delta) VALUES(0)";
	inline static const char update_delta_sql[] = "UPDATE time SET delta = ?";
};

}
