#include <cdedent.hpp>
#include <raylib.h>

#include "draw_systems.hpp"
#include "move_vector.hpp"
#include "spawn_moving_object.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/texture_flyweight.hpp"

void register_spawn_moving_object(ecsql::Ecsql& world) {
	world.register_system({
		"SpawnMovingObject",
		[](PreparedSQL& spawner_position, PreparedSQL& update_spawn_time, PreparedSQL& insert_entity, PreparedSQL& insert_position, PreparedSQL& insert_move_vector, PreparedSQL& insert_texture) {
			if (!IsKeyDown(KEY_SPACE)) {
				return;
			}

			for (ecsql::SQLRow row : spawner_position()) {
				auto [entity, position, direction, texture_path] = row.get<Entity, Vector3, Vector2, std::string_view>();
				update_spawn_time(entity);

				Entity spawned_entity = insert_entity().get<Entity>();
				insert_position(spawned_entity, position);
				insert_move_vector(spawned_entity, direction);
				insert_texture(spawned_entity, texture_path);
			}
		},
		R"(
			SELECT entity_id, x, y, z, direction_x, direction_y, texture_path
			FROM Spawner
			  JOIN Position USING(entity_id)
			  JOIN time
			WHERE last_spawn_time IS NULL
			  OR last_spawn_time < time.uptime - cooldown
		)"_dedent,
		R"(
			UPDATE Spawner
			SET last_spawn_time = time.uptime
			FROM time
			WHERE entity_id = ?
		)"_dedent,
		Entity::insert_sql,
		PositionComponent.insert_sql(),
		MoveVector.insert_sql(),
		TextureFlyweight.component.insert_sql(),
	});
}
