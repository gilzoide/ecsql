#pragma once

typedef struct sqlite3 sqlite3;

void register_sqlite_functions(sqlite3 *db);
