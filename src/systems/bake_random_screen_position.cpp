#include <cdedent.hpp>

#include "bake_random_screen_position.hpp"
#include "../ecsql/system.hpp"

void register_bake_random_screen_position_system(ecsql::World& world) {
	world.register_system({
		"BakeRandomScreenPosition",
		{
			R"(
				INSERT OR REPLACE INTO Position(entity_id, x, y)
				SELECT
					entity_id,
					abs(random()) % (screen_size.width - paddingLeft - paddingRight) + paddingLeft AS x,
					abs(random()) % (screen_size.height - paddingTop - paddingBottom) + paddingTop AS y
				FROM BakeRandomScreenPosition
				JOIN screen_size
			)"_dedent,
			"DELETE FROM BakeRandomScreenPosition",
		},
		[](auto sqls) {
			auto bake = sqls[0];
			auto delete_bakers = sqls[1];

			bake();
			delete_bakers();
		},
	});
}
