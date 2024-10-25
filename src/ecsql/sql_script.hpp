#pragma once

#include <sqlite3.h>

namespace ecsql {

void execute_sql_script(sqlite3 *db, const char *sql);

}
