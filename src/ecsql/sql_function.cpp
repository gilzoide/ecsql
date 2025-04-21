#include "sql_function.hpp"

namespace ecsql {

void set_function_result(sqlite3_context *ctx, bool value) {
	sqlite3_result_int(ctx, value);
}

void set_function_result(sqlite3_context *ctx, sqlite3_int64 value) {
	sqlite3_result_int64(ctx, value);
}

void set_function_result(sqlite3_context *ctx, double value) {
	sqlite3_result_double(ctx, value);
}

void set_function_result(sqlite3_context *ctx, const char *value) {
	sqlite3_result_text(ctx, value, -1, SQLITE_TRANSIENT);
}

void set_function_result(sqlite3_context *ctx, std::string_view value) {
	sqlite3_result_text(ctx, value.data(), value.length(), SQLITE_TRANSIENT);
}

void set_function_result(sqlite3_context *ctx, const std::string& value) {
	sqlite3_result_text(ctx, value.data(), value.length(), SQLITE_TRANSIENT);
}

}
