#include <cdedent.hpp>
#include <string_view>

#include "spawn_at_most.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/scene_sql_flyweight.hpp"

void register_spawn_at_most(ecsql::World& world) {
	world.register_system({
		"SpawnAtMost",
		{
			R"(
				SELECT
					SceneSql.path,
					SpawnAtMost.count - ifnull(count(entity.name), 0)
				FROM SpawnAtMost
					JOIN SceneSql USING(entity_id)
					LEFT JOIN entity ON entity.name = SpawnAtMost.name
				GROUP BY SpawnAtMost.name
			)"_dedent,
		},
		[](ecsql::World& world, std::vector<ecsql::PreparedSQL>& sqls) {
			auto select_spawn = sqls[0];

			for (ecsql::SQLRow row : select_spawn()) {
				auto [scene_path, spawn_count] = row.get<std::string_view, int>();
				if (spawn_count <= 0) {
					continue;
				}

				auto scene_sql = SceneSqlFlyweight.get(scene_path);
				for (int i = 0; i < spawn_count; i++) {
					world.execute_sql_script(scene_sql.value.c_str());
				}
			}
		},
	});
}
