#pragma once

#include <string_view>

namespace ecsql {

struct time {
	// Schema
	float delta;
	float uptime;

	// SQL statements
	inline static const char schema_sql[] = "CREATE TABLE time(delta, uptime)";
	inline static const char insert_singleton_sql[] = "INSERT INTO time(delta, uptime) VALUES(0, 0)";
	inline static const char update_delta_sql[] = "UPDATE time SET delta = ?1, uptime = uptime + ?1";
};

}
