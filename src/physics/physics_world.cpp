#include <unordered_set>

#include <box2d/box2d.h>
#include <cdedent.hpp>

#include "physics_body.hpp"
#include "physics_world.hpp"
#include "../ecsql/system.hpp"

std::unordered_map<ecsql::EntityID, b2WorldId> world_map;

ecsql::Component WorldComponent {
	"World",
	{
		// b2WorldDef variables
		"gravity_x",
		"gravity_y",
		"restitution_threshold",
		"hit_event_threshold",
		"contact_hertz",
		"contact_damping_ratio",
		"contact_push_max_speed",
		"joint_hertz",
		"joint_damping_ratio",
		"maximum_linear_speed",
		"enable_sleep",
		"enable_continuous",
		// Simulation options
		"timestep NOT NULL DEFAULT (1.0 / 60.0)",
		"substep_count NOT NULL DEFAULT 4",
		// Internal variables
		"time_accumulator"
	}
};

ecsql::HookSystem WorldHookSystem {
	WorldComponent,
	[](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
		switch (hook) {
			case ecsql::HookType::OnInsert: {
				auto [
					entity_id,
					gravity,
					restitution_threshold,
					hit_event_threshold,
					contact_hertz, contact_damping_ratio, contact_push_max_speed,
					joint_hertz, joint_damping_ratio,
					maximum_linear_speed,
					enable_sleep, enable_continuous
				] = new_row.get<
					ecsql::EntityID,
					std::optional<b2Vec2>,
					std::optional<float>,
					std::optional<float>,
					std::optional<float>, std::optional<float>, std::optional<float>,
					std::optional<float>, std::optional<float>,
					std::optional<float>,
					std::optional<bool>, std::optional<bool>
				>();
				b2WorldDef worlddef = b2DefaultWorldDef();
				if (gravity) {
					worlddef.gravity = *gravity;
				}
				if (restitution_threshold) {
					worlddef.restitutionThreshold = *restitution_threshold;
				}
				if (hit_event_threshold) {
					worlddef.hitEventThreshold = *hit_event_threshold;
				}
				if (contact_hertz) {
					worlddef.contactHertz = *contact_hertz;
				}
				if (contact_damping_ratio) {
					worlddef.contactDampingRatio = *contact_damping_ratio;
				}
				if (contact_push_max_speed) {
					worlddef.contactPushMaxSpeed = *contact_push_max_speed;
				}
				if (joint_hertz) {
					worlddef.jointHertz = *joint_hertz;
				}
				if (joint_damping_ratio) {
					worlddef.jointDampingRatio = *joint_damping_ratio;
				}
				if (maximum_linear_speed) {
					worlddef.maximumLinearSpeed = *maximum_linear_speed;
				}
				if (enable_sleep) {
					worlddef.enableSleep = *enable_sleep;
				}
				if (enable_continuous) {
					worlddef.enableContinuous = *enable_continuous;
				}
				b2WorldId world_id = b2CreateWorld(&worlddef);
				world_map.emplace(entity_id, world_id);
				break;
			}

			case ecsql::HookType::OnUpdate:
				break;

			case ecsql::HookType::OnDelete: {
				auto it = world_map.find(old_row.get<ecsql::EntityID>(0));
				if (it != world_map.end()) {
					if (b2World_IsValid(it->second)) {
						b2DestroyWorld(it->second);
					}
					world_map.erase(it);
				}
				break;
			}
		}
	}
};

void register_physics_world(ecsql::World& world) {
	world.register_system({
		"physics.UpdateWorld",
		{
			R"(
				SELECT
					entity_id,
					timestep, substep_count,
					delta, time_accumulator
				FROM World
					JOIN time
			)"_dedent,
			R"(
				UPDATE World
				SET time_accumulator = ?2
				WHERE entity_id = ?1
			)"_dedent,
			R"(
				REPLACE INTO Position(entity_id, x, y)
				VALUES(?, ?, ?)
			)"_dedent,
			R"(
				REPLACE INTO Rotation(entity_id, z)
				VALUES(?, ?)
			)"_dedent,
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto get_worlds = sqls[0];
			auto update_time_accumulator = sqls[1];
			auto update_position = sqls[2];
			auto update_rotation = sqls[3];
			for (auto row : get_worlds()) {
				auto [
					world_entity_id,
					timestep, substep_count,
					delta, time_accumulator
				] = row.get<
					ecsql::EntityID,
					float, int,
					float, float
				>();

				if (timestep <= 0) {
					continue;
				}

				// Simulate world
				static std::unordered_set<BodyUserData *> moved_bodies;
				moved_bodies.clear();

				b2WorldId world_id = world_map.at(world_entity_id);
				time_accumulator += delta;
				while (time_accumulator >= timestep) {
					b2World_Step(world_id, timestep, substep_count);
					time_accumulator -= timestep;

					b2BodyEvents body_events = b2World_GetBodyEvents(world_id);
					for (auto move_event : std::span<b2BodyMoveEvent>(body_events.moveEvents, body_events.moveCount)) {
						BodyUserData *user_data = BodyUserData::from(move_event.bodyId);
						user_data->update_transform(move_event.transform);
						moved_bodies.insert(user_data);
					}
				}
				update_time_accumulator(world_entity_id, time_accumulator);

				// Update body data
				float alpha = time_accumulator / timestep;
				for (auto user_data : moved_bodies) {
					b2Transform interpolated_transform = user_data->interpolated_transform(alpha);
					update_position(user_data->entity_id, interpolated_transform.p);
					float rotation = b2Rot_GetAngle(interpolated_transform.q);
					update_rotation(user_data->entity_id, rotation);
				}
			}
		},
	});
}
