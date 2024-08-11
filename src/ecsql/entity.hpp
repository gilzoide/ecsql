#pragma once

#include <sqlite3.h>

struct Entity {
    Entity();
    Entity(sqlite3_int64 id);

    sqlite3_int64 id;
};
