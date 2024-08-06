#pragma once

#include <functional>
#include <initializer_list>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <sqlite3.h>
#include <vector>

#include "component.hpp"
#include "PreparedSQL.hpp"

typedef struct sqlite3 sqlite3;
typedef sqlite3_int64 entity_id;

namespace ecsql {

class Component;
class SQLRow;

class Ecsql {
public:
    Ecsql();
    Ecsql(const char *db_name);
    Ecsql(sqlite3 *db);
    ~Ecsql();

    template<typename T> void register_component() {
        register_component(Component::from_type<T>());
    }
    template<typename T> void register_component(const std::string& name) {
        register_component(Component::from_type<T>(name));
    }
    void register_component(const Component& component);
    void register_system(const std::string& name, const std::string& query, std::function<void(const SQLRow&)> f);

    entity_id create_entity();
    bool delete_entity(entity_id id);

    void inside_transaction(std::function<void()> f);
    void inside_transaction(std::function<void(Ecsql&)> f);
    void inside_transaction(std::function<void(sqlite3 *)> f);

    void update();

private:
    sqlite3 *db;
    PreparedSQL begin_stmt;
    PreparedSQL commit_stmt;
    PreparedSQL rollback_stmt;
    PreparedSQL create_entity_stmt;
    PreparedSQL delete_entity_stmt;

    std::vector<std::function<void()>> systems;
};

}
