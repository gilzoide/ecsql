#pragma once

#include <string_view>

#include <sqlite3.h>

namespace ecsql {

// Deleter for smart pointers that call `sqlite3_free`
struct sqlite3_free_deleter {
	void operator()(void *ptr) {
		sqlite3_free(ptr);
	}
};

// Deleter for smart pointers that call `sqlite3_close_v2`
struct sqlite3_close_v2_deleter {
	void operator()(sqlite3 *ptr) {
		sqlite3_close_v2(ptr);
	}
};

void execute_sql_script(sqlite3 *db, const char *sql);
std::string_view extract_identifier(std::string_view field);

}
