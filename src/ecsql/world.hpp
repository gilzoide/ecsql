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
#include "hook_system.hpp"
#include "prepared_sql.hpp"

namespace ecsql {

class Component;
class SQLRow;
class System;

class World {
public:
    World();
    World(const char *db_name);
    ~World();

    void register_component(Component& component);
    void register_component(Component&& component);

    void register_system(System& system);
    void register_system(System&& system);

	void register_hook_system(const HookSystem& system);
    void register_hook_system(HookSystem&& system);

    EntityID create_entity();
    EntityID create_entity(std::string_view name);
    EntityID create_entity(std::string_view name, EntityID parent);
    EntityID create_entity(EntityID parent);
    bool delete_entity(EntityID id);

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

    void update(float time_delta);

	void on_window_resized(int new_width, int new_height);

    bool backup_into(const char *db_name);
    bool backup_into(sqlite3 *db);

    bool restore_from(const char *db_name);
    bool restore_from(sqlite3 *db);

    std::shared_ptr<sqlite3> get_db() const;
	void execute_sql_script(const char *sql);

	template<typename... Args>
	ExecutedSQL execute_sql(std::string_view sql, Args&&... args) {
		return PreparedSQL(db.get(), sql, false)(std::forward<Args>(args)...);
	}

private:
    std::shared_ptr<sqlite3> db;
    PreparedSQL begin_stmt;
    PreparedSQL commit_stmt;
    PreparedSQL rollback_stmt;
    PreparedSQL create_entity_stmt;
    PreparedSQL delete_entity_stmt;
    PreparedSQL update_delta_time_stmt;

    std::vector<System> systems;
    std::unordered_map<std::string, std::vector<HookSystem>> hook_systems;

	static void preupdate_hook(void *pCtx, sqlite3 *db, int op, char const *zDb, char const *zName, sqlite3_int64 iKey1, sqlite3_int64 iKey2);
    void execute_prehook(const char *table, HookType hook, sqlite3_int64 old_rowid, sqlite3_int64 new_rowid);
};

}
