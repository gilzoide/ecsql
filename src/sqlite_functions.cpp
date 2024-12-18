#include "sqlite_functions.hpp"
#include "ecsql/sql_value.hpp"
#include "ecsql/world.hpp"

void clamp_function(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
	ecsql::SQLValue value(argv[0]);
	ecsql::SQLValue min(argv[1]);
	ecsql::SQLValue max(argv[2]);

	double result = std::clamp(value.get_double(), min.get_double(), max.get_double());
	sqlite3_result_double(ctx, result);
}

void register_sqlite_functions(ecsql::World& world) {
	world.create_function("clamp", 3, clamp_function);
}
