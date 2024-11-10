#include <cdedent.hpp>
#include <raylib.h>

#include "draw_systems.hpp"
#include "move_vector.hpp"
#include "spawn_scene_on_key.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/scene_sql_flyweight.hpp"
#include "../flyweights/texture_flyweight.hpp"

void register_spawn_scene_on_key(ecsql::Ecsql& world) {
	world.register_system({
		"SpawnPrefabObject",
		{
			R"(
				SELECT entity_id, SceneSql.path
				FROM SpawnOnKey
					JOIN SceneSql USING(entity_id)
					JOIN time
				WHERE last_spawn_time IS NULL
					OR last_spawn_time < time.uptime - cooldown
			)"_dedent,
			R"(
				UPDATE SpawnOnKey
				SET last_spawn_time = time.uptime
				FROM time
				WHERE last_spawn_time IS NULL
					OR last_spawn_time < time.uptime - cooldown
			)"_dedent,
			R"(
				UPDATE BakePosition
				SET parent_id = ?
				WHERE entity_id = ?
			)"_dedent,
		},
		[](Ecsql& world, std::vector<PreparedSQL>& sqls) {
			if (!IsKeyDown(KEY_SPACE)) {
				return;
			}

			auto get_spawn_info = sqls[0];
			auto update_spawn_time = sqls[1];
			auto update_bake_position_parent = sqls[2];

			for (ecsql::SQLRow row : get_spawn_info()) {
				auto [entity, scene_path] = row.get<EntityID, std::string_view>();
				
				auto scene_sql = SceneSqlFlyweight.get(scene_path);
				world.execute_sql_script(scene_sql.value.c_str());

				EntityID spawned_entity = sqlite3_last_insert_rowid(world.get_db().get());
				update_bake_position_parent(entity, spawned_entity);
			}
			update_spawn_time();
		},
	});
}
