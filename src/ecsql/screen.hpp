#pragma once

namespace ecsql {

struct screen {
	// Schema
	int width;
	int height;

	// SQL statements
	inline static const char update_sql[] = "UPDATE screen SET width = ?, height = ?";
};

}
