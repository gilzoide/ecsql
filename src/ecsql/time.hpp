#pragma once

namespace ecsql {

struct time {
	// Schema
	float delta;
	float uptime;

	// SQL statements
	inline static const char update_delta_sql[] = "UPDATE time SET delta = ?1, uptime = uptime + ?1";
	inline static const char select_fixed_delta_time_sql[] = "SELECT fixed_delta FROM time";
	inline static const char update_fixed_delta_progress_sql[] = "UPDATE time SET fixed_delta_progress = ?";
};

}
