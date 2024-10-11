#include "entity.hpp"

Entity::Entity()
    : id(0)
{
}

Entity::Entity(sqlite3_int64 id)
    : id(id)
{
}

const char *Entity::schema_sql() {
    return "CREATE TABLE entity(id INTEGER PRIMARY KEY, name TEXT);\n"
        "CREATE INDEX entity_name ON entity(name);";
}
