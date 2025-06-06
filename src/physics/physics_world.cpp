#include <box2d/box2d.h>
#include <cdedent.hpp>
#include <raylib.h>

#include "physics_body.hpp"
#include "physics_shape.hpp"
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
		"max_contact_push_speed",
		"joint_hertz",
		"joint_damping_ratio",
		"maximum_linear_speed",
		"enable_sleep",
		"enable_continuous",
		// Simulation options
		"substep_count NOT NULL DEFAULT 4",
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
					contact_hertz, contact_damping_ratio, max_contact_push_speed,
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
				if (max_contact_push_speed) {
					worlddef.maxContactPushSpeed = *max_contact_push_speed;
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
					fixed_delta, substep_count
				FROM World
					JOIN time
			)"_dedent,
			R"(
				REPLACE INTO Position(entity_id, x, y)
				VALUES(?, ?, ?)
			)"_dedent,
			R"(
				REPLACE INTO Rotation(entity_id, z)
				VALUES(?, ?)
			)"_dedent,
			R"(
				REPLACE INTO LinearVelocity(entity_id, x, y)
				VALUES(?, ?, ?)
			)"_dedent,
			R"(
				REPLACE INTO AngularVelocity(entity_id, z)
				VALUES(?, ?)
			)"_dedent,
			R"(
				INSERT INTO Contact(
					entity_id,
					shape1, shape2,
					normal_x, normal_y
				)
				VALUES(?, ?, ?, ?, ?)
			)"_dedent,
			R"(
				DELETE FROM Contact
				WHERE shape1 = ?1 AND shape2 = ?2
			)"_dedent,
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto get_worlds = sqls[0];
			auto update_position = sqls[1];
			auto update_rotation = sqls[2];
			auto update_linear_velocity = sqls[3];
			auto update_angular_velocity = sqls[4];
			auto insert_contact = sqls[5];
			auto delete_contact = sqls[6];
			for (auto row : get_worlds()) {
				auto [
					world_entity_id,
					timestep, substep_count
				] = row.get<
					ecsql::EntityID,
					float, int
				>();

				// Simulate world
				b2WorldId world_id = world_map.at(world_entity_id);
				b2World_Step(world_id, timestep, substep_count);

				// Update body positions
				b2BodyEvents body_events = b2World_GetBodyEvents(world_id);
				for (auto move_event : std::span<b2BodyMoveEvent>(body_events.moveEvents, body_events.moveCount)) {
					ecsql::EntityID entity_id = BodyUserData::from(move_event.bodyId)->entity_id;

					update_position(entity_id, move_event.transform.p);
					update_rotation(entity_id, b2Rot_GetAngle(move_event.transform.q) * RAD2DEG);

					update_linear_velocity(entity_id, b2Body_GetLinearVelocity(move_event.bodyId));
					update_angular_velocity(entity_id, b2Body_GetAngularVelocity(move_event.bodyId) * RAD2DEG);
				}

				// Update contacts
				b2ContactEvents contact_events = b2World_GetContactEvents(world_id);
				for (auto begin_contacts : std::span<b2ContactBeginTouchEvent>(contact_events.beginEvents, contact_events.beginCount)) {
					insert_contact(
						world_entity_id,
						ShapeUserData::from(begin_contacts.shapeIdA)->entity_id,
						ShapeUserData::from(begin_contacts.shapeIdB)->entity_id,
						begin_contacts.manifold.normal.x,
						begin_contacts.manifold.normal.y
					);
				}
				for (auto end_contacts : std::span<b2ContactEndTouchEvent>(contact_events.endEvents, contact_events.endCount)) {
					if (b2Shape_IsValid(end_contacts.shapeIdA) && b2Shape_IsValid(end_contacts.shapeIdB)) {
						delete_contact(
							ShapeUserData::from(end_contacts.shapeIdA)->entity_id,
							ShapeUserData::from(end_contacts.shapeIdB)->entity_id
						);
					}
				}
			}
		},
	}, true);
}
