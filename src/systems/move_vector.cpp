#include <cdedent.hpp>
#include <raylib.h>

#include "move_vector.hpp"
#include "../ecsql/system.hpp"

void register_move_vector(ecsql::World& world) {
	world.register_system({
		"MoveVector",
		R"(
			UPDATE Position
			SET x = Position.x + movement.x, y = Position.y + movement.y
			FROM (
				SELECT entity_id, ifnull(MoveVector.x, 0) * time.delta AS x, ifnull(MoveVector.y, 0) * time.delta AS y
				FROM MoveVector, time
			) AS movement
			WHERE Position.entity_id = movement.entity_id
		)"_dedent,
		[](auto& update_position) {
			update_position();
		},
	});
}
