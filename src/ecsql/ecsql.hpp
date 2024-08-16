#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <sqlite3.h>

#include "entity.hpp"
#include "prepared_sql.hpp"

namespace ecsql {

class Component;
class HookSystem;
class System;
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
    
    void register_on_insert_system(const HookSystem& system);
    void register_on_insert_system(HookSystem&& system);
    
    void register_on_delete_system(const HookSystem& system);
    void register_on_delete_system(HookSystem&& system);
    
    void register_on_update_system(const HookSystem& system);
    void register_on_update_system(HookSystem&& system);

    Entity create_entity();
    Entity create_entity(std::string_view name);
    bool delete_entity(Entity id);

    void inside_transaction(std::function<void()> f);
    void inside_transaction(std::function<void(Ecsql&)> f);

    void update();
    void on_insert(const char *table);
    void on_delete(const char *table);
    void on_update(const char *table);

    sqlite3 *get_db() const;

private:
    sqlite3 *db;
    PreparedSQL begin_stmt;
    PreparedSQL commit_stmt;
    PreparedSQL rollback_stmt;
    PreparedSQL create_entity_stmt;
    PreparedSQL delete_entity_stmt;

    std::vector<System> systems;
    std::unordered_map<std::string, std::vector<HookSystem>> on_insert_systems;
    std::unordered_map<std::string, std::vector<HookSystem>> on_delete_systems;
    std::unordered_map<std::string, std::vector<HookSystem>> on_update_systems;

    void register_prehook(std::unordered_map<std::string, std::vector<HookSystem>>& map, const HookSystem& system);
    void register_prehook(std::unordered_map<std::string, std::vector<HookSystem>>& map, HookSystem&& system);
    void execute_prehook(const char *table, const std::unordered_map<std::string, std::vector<HookSystem>>& map);
};

}
