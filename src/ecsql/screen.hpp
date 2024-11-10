#pragma once

namespace ecsql {

struct screen_size {
	// Schema
	int width;
	int height;

	// SQL statements
	inline static const char update_sql[] = "UPDATE screen_size SET width = ?, height = ?";
};

}
