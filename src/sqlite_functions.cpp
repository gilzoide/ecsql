#include <raylib.h>
#include <raymath.h>

#include "sqlite_functions.hpp"
#include "ecsql/sql_function.hpp"

void register_sqlite_functions(sqlite3 *db) {
	ecsql::register_sql_function(db, Clamp, "clamp", SQLITE_UTF8 | SQLITE_DETERMINISTIC | SQLITE_INNOCUOUS);
	ecsql::register_sql_function(db, IsKeyReleased, "IsKeyReleased", SQLITE_UTF8 | SQLITE_INNOCUOUS);
}
