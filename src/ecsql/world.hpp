#pragma once

#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <dispatch_queue.hpp>
#include <sqlite3.h>
#include <tracy/Tracy.hpp>

#include "entity.hpp"
#include "executed_sql.hpp"
#include "fixed_delta_executor.hpp"
#include "hook_system.hpp"
#include "prepared_sql.hpp"

namespace ecsql {

class BackgroundSystem;
class Component;
class SQLRow;
class System;

class World {
public:
	World();
	World(const char *world_db_path);
	World(const char *world_db_path, const char *save_db_path);
	~World();

	void register_component(const Component& component);
	void register_component(Component&& component);

	void register_system(const System& system, bool use_fixed_delta = false);
	void register_system(System&& system, bool use_fixed_delta = false);
	void remove_system(std::string_view system_name);
	void remove_system(const System& system);
	void remove_systems_with_prefix(std::string_view system_name_prefix);

	void register_hook_system(const HookSystem& system);
	void register_hook_system(HookSystem&& system);

	void register_background_system(const BackgroundSystem& system);
	void register_background_system(BackgroundSystem&& system);
	void remove_background_system(std::string_view system_name);
	void remove_background_system(const BackgroundSystem& system);
	void remove_background_systems_with_prefix(std::string_view system_name_prefix);

	EntityID create_entity(std::optional<std::string_view> name = std::nullopt, std::optional<EntityID> parent = std::nullopt);
	std::optional<EntityID> find_entity(std::string_view name);
	int delete_entity(EntityID id);
	int delete_entity(std::string_view name);

	template<typename Fn>
	bool inside_transaction(Fn&& f) {
		ZoneScoped;
		begin_transaction();
		try {
			if constexpr (std::is_invocable_v<Fn>) {
				f();
			}
			else {
				f(*this);
			}
			commit_transaction();
			return true;
		}
		catch (std::runtime_error& err) {
			std::cerr << "Runtime error: " << err.what() << std::endl;
			rollback_transaction();

			return false;
		}
	}

	void begin_transaction();
	void commit_transaction();
	void rollback_transaction();

	void update(float time_delta);

	void on_window_resized(int new_width, int new_height);

	bool backup_into(const char *filename, const char *db_name = "main");
	bool backup_into(sqlite3 *db, const char *db_name = "main");

	bool restore_from(const char *filename, const char *db_name = "main");
	bool restore_from(sqlite3 *db, const char *db_name = "main");

	std::shared_ptr<sqlite3> get_db() const;
	PreparedSQL prepare_sql(std::string_view sql, bool is_persistent = false);
	void execute_sql_script(const char *sql);

	template<typename... Args>
	ExecutedSQL execute_sql(std::string_view sql, Args&&... args) {
		return prepare_sql(sql)(std::forward<Args>(args)...);
	}

	void create_function(const char *name, int argument_count, void (*fn)(sqlite3_context*, int, sqlite3_value**));

private:
	std::shared_ptr<sqlite3> db;
	PreparedSQL begin_stmt;
	PreparedSQL commit_stmt;
	PreparedSQL rollback_stmt;
	PreparedSQL create_entity_stmt;
	PreparedSQL delete_entity_stmt;
	PreparedSQL delete_entity_by_name_stmt;
	PreparedSQL find_entity_stmt;
	PreparedSQL update_delta_time_stmt;
	PreparedSQL select_fixed_delta_time_stmt;
	PreparedSQL update_fixed_delta_progress_stmt;

	std::vector<std::tuple<System, std::vector<PreparedSQL>>> systems;
	std::vector<std::tuple<System, std::vector<PreparedSQL>>> fixed_systems;
	std::unordered_map<std::string, std::vector<HookSystem>> hook_systems;
	std::vector<std::pair<BackgroundSystem, std::future<void>>> background_systems;

	dispatch_queue::dispatch_queue dispatch_queue;
	std::future<void> commit_or_rollback_result;

	fixed_delta_executor fixed_delta_executor;

	static void preupdate_hook(void *pCtx, sqlite3 *db, int op, char const *zDb, char const *zName, sqlite3_int64 iKey1, sqlite3_int64 iKey2);
	void execute_prehook(const char *table, HookType hook, sqlite3_int64 old_rowid, sqlite3_int64 new_rowid);
	void execute_all_prehooks(HookType hook);
};

}
