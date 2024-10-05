#pragma once

#include <iostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <sqlite3.h>

#include "entity.hpp"
#include "executed_sql.hpp"
#include "prepared_sql.hpp"

namespace ecsql {

class RawComponent;
class HookSystem;
class System;
class SQLRow;

class Ecsql {
public:
    Ecsql();
    Ecsql(const char *db_name);
    Ecsql(sqlite3 *db);
    ~Ecsql();

    void register_component(RawComponent& component);
    void register_component(RawComponent&& component);
    
    void register_system(System& system);
    void register_system(System&& system);
    
	void register_hook_system(const HookSystem& system);
    void register_hook_system(HookSystem&& system);

    Entity create_entity();
    Entity create_entity(std::string_view name);
    bool delete_entity(Entity id);

    template<typename Fn>
    bool inside_transaction(Fn&& f) {
        begin_stmt();
        try {
            f(*this);
            commit_stmt();
			return true;
        }
        catch (std::runtime_error& err) {
            std::cerr << "Runtime error: " << err.what() << std::endl;
            rollback_stmt();
			return false;
        }
    }

    void update();
    void on_insert(const char *table);
    void on_delete(const char *table);
    void on_update(const char *table);

    bool backup_into(const char *db_name);
    bool backup_into(sqlite3 *db);
    
    bool restore_from(const char *db_name);
    bool restore_from(sqlite3 *db);

	bool load_scene(std::istream& stream, std::string_view source_path = {});
	bool load_scene_file(const char *file_name);

    sqlite3 *get_db() const;

	template<typename... Args>
	ExecutedSQL execute_sql(std::string_view sql, Args&&... args) {
		return PreparedSQL(db, sql, false)(std::forward<Args>(args)...);
	}

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
