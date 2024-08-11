#pragma once

#include <functional>
#include <initializer_list>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <sqlite3.h>
#include <vector>

#include "entity.hpp"
#include "component.hpp"
#include "PreparedSQL.hpp"
#include "system.hpp"

namespace ecsql {

class Component;
class SQLRow;

class Ecsql {
public:
    Ecsql();
    Ecsql(const char *db_name);
    Ecsql(sqlite3 *db);
    ~Ecsql();

    void register_component(Component& component);
    void register_component(Component&& component);
    void register_system(System& system);
    void register_system(System&& system);

    Entity create_entity();
    bool delete_entity(Entity id);

    void inside_transaction(std::function<void()> f);
    void inside_transaction(std::function<void(Ecsql&)> f);

    void update();

    sqlite3 *get_db() const;

private:
    sqlite3 *db;
    PreparedSQL begin_stmt;
    PreparedSQL commit_stmt;
    PreparedSQL rollback_stmt;
    PreparedSQL create_entity_stmt;
    PreparedSQL delete_entity_stmt;

    std::vector<System> systems;
};

}
