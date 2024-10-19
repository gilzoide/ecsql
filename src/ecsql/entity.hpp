#pragma once

#include <sqlite3.h>

namespace ecsql {

struct Entity {
    Entity();
    Entity(sqlite3_int64 id);

    static const char *schema_sql();

    sqlite3_int64 id;
};
	
}

