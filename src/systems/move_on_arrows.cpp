#include <raylib.h>
#include <raymath.h>

#include "move_on_arrows.hpp"
#include "../ecsql/system.hpp"

void register_move_on_arrows(ecsql::Ecsql& world) {
	world.register_system({
		"MoveOnArrows",
		[](auto& update_position) {
			bool going_up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
			bool going_down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
			bool going_left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
			bool going_right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
			Vector2 movement = {
				(float) going_right - (float) going_left,
				(float) going_down - (float) going_up,
			};
			movement = Vector2Normalize(movement);
			if (movement.x || movement.y) {
				update_position(movement.x, movement.y);
			}
		},
		R"(
			UPDATE Position
			SET x = x + ? * movement.speed, y = y + ? * movement.speed
			FROM (
				SELECT entity_id, ifnull(LinearSpeed.value, 1) AS speed
				FROM MoveOnArrows
				LEFT JOIN LinearSpeed USING(entity_id)
			) AS movement
			WHERE Position.entity_id = movement.entity_id
		)",
	});
}