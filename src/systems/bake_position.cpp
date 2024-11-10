#include <cdedent.hpp>

#include "bake_position.hpp"
#include "../ecsql/system.hpp"

void register_bake_position_system(ecsql::Ecsql& world) {
	world.register_system({
		"BakeRelativePosition",
		{
			R"(
				INSERT OR REPLACE INTO Position(entity_id, x, y, z)
				SELECT
					BakePosition.entity_id,
					BakePosition.x + ParentPosition.x,
					BakePosition.y + ParentPosition.y,
					BakePosition.z + ParentPosition.z
				FROM BakePosition
					JOIN Position AS ParentPosition ON BakePosition.parent_id = ParentPosition.entity_id
			)"_dedent,
			"DELETE FROM BakePosition",
		},
		[](auto& sqls) {
			auto bake_position = sqls[0];
			auto delete_bake_component = sqls[1];
			bake_position();
			delete_bake_component();
		},
	});
}
