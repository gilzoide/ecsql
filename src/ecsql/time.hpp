#pragma once

namespace ecsql {

struct time {
	// Schema
	float delta;
	float uptime;

	// SQL statements
	inline static const char update_delta_sql[] = "UPDATE time SET delta = ?1, uptime = uptime + ?1";
};

}
