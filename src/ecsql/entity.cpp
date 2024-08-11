#include "entity.hpp"

Entity::Entity()
    : id(0)
{
}

Entity::Entity(sqlite3_int64 id)
    : id(id)
{
}
