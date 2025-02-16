#include <cdedent.hpp>
#include <box2d/box2d.h>

#include "physics.hpp"
#include "../ecsql/system.hpp"
#include "../ecsql/hook_system.hpp"

std::unordered_map<ecsql::EntityID, b2WorldId> world_map;
std::unordered_map<ecsql::EntityID, b2BodyId> body_map;
std::vector<ecsql::EntityID> pending_create_body;

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

ecsql::Component BodyComponent {
	"Body",
	{
		"world REFERENCES World(entity_id) ON DELETE CASCADE",
		"type",  // "static", "kinematic", "dynamic"
		"linear_damping",
		"angular_damping",
		"gravity_scale",
		"sleep_threshold",
		"enable_sleep",
		"is_awake",
		"fixed_rotation",
		"is_bullet",
		"is_enabled",
		"allow_fast_rotation",
	}
};

ecsql::HookSystem BodyHookSystem {
	BodyComponent,
	[](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
		switch (hook) {
			case ecsql::HookType::OnInsert:
				pending_create_body.push_back(new_row.get<ecsql::EntityID>(0));
				break;

			case ecsql::HookType::OnUpdate:
				break;

			case ecsql::HookType::OnDelete: {
				auto it = body_map.find(old_row.get<ecsql::EntityID>(0));
				if (it != body_map.end()) {
					if (b2Body_IsValid(it->second)) {
						b2DestroyBody(it->second);
					}
					body_map.erase(it);
				}
				break;
			}
		}
	},
};

Physics::Physics(ecsql::World& world)
	: ecs_world(world)
{
	world.register_system({
		"physics.CreateBody",
		{
			R"(
				SELECT
					-- entity
					name,
					-- body
					world,
					type,
					linear_damping,
					angular_damping,
					gravity_scale,
					sleep_threshold,
					enable_sleep,
					is_awake,
					fixed_rotation,
					is_bullet,
					is_enabled,
					allow_fast_rotation,
					-- position
					Position.x, Position.y,
					-- rotation
					Rotation.z
				FROM Body
					JOIN entity ON Body.entity_id = entity.id
					LEFT JOIN Position USING(entity_id)
					LEFT JOIN Rotation USING(entity_id)
				WHERE entity_id = ?
			)"_dedent,
			R"(
				SELECT entity_id
				FROM World
				LIMIT 1
			)"_dedent,
		},
		[this](std::vector<ecsql::PreparedSQL>& sqls) {
			auto select_body = sqls[0];
			auto get_default_world = sqls[1];

			b2WorldId default_world = b2_nullWorldId;
			for (auto entity_id : pending_create_body) {
				auto [
					name,
					world_entity_id,
					type,
					linear_damping, angular_damping, gravity_scale, sleep_threshold,
					enable_sleep, is_awake, fixed_rotation, is_bullet, is_enabled, allow_fast_rotation,
					position,
					rotation_angle
				] = select_body(entity_id).get<
					std::optional<const char *>,
					std::optional<ecsql::EntityID>,
					std::optional<std::string_view>,
					std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
					std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>,
					std::optional<b2Vec2>,
					std::optional<float>
				>();

				b2WorldId world_id = b2_nullWorldId;
				if (world_entity_id) {
					world_id = world_map.at(*world_entity_id);
				}
				else if (b2World_IsValid(default_world)) {
					world_id = default_world;
				}
				else {
					auto world_entity_id = get_default_world().get<ecsql::EntityID>();
					if (world_entity_id) {
						world_id = default_world = world_map.at(world_entity_id);
					}
				}

				if (!b2World_IsValid(world_id)) {
					continue;
				}

				b2BodyDef bodydef = b2DefaultBodyDef();
				if (name) {
					bodydef.name = *name;
				}
				if (type == "static") {
					bodydef.type = b2_staticBody;
				}
				else if (type == "kinematic") {
					bodydef.type = b2_kinematicBody;
				}
				else {
					bodydef.type = b2_dynamicBody;
				}
				if (linear_damping) {
					bodydef.linearDamping = *linear_damping;
				}
				if (angular_damping) {
					bodydef.angularDamping = *angular_damping;
				}
				if (gravity_scale) {
					bodydef.gravityScale = *gravity_scale;
				}
				if (sleep_threshold) {
					bodydef.sleepThreshold = *sleep_threshold;
				}
				if (enable_sleep) {
					bodydef.enableSleep = *enable_sleep;
				}
				if (is_awake) {
					bodydef.isAwake = *is_awake;
				}
				if (fixed_rotation) {
					bodydef.fixedRotation = *fixed_rotation;
				}
				if (is_bullet) {
					bodydef.isBullet = *is_bullet;
				}
				if (is_enabled) {
					bodydef.isEnabled = *is_enabled;
				}
				if (allow_fast_rotation) {
					bodydef.allowFastRotation = *allow_fast_rotation;
				}
				if (position) {
					bodydef.position = *position;
				}
				if (rotation_angle) {
					bodydef.rotation = b2MakeRot(*rotation_angle);
				}
				b2BodyId body_id = b2CreateBody(world_id, &bodydef);
				body_map[entity_id] = body_id;
			}
			pending_create_body.clear();
		},
	});
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
		},
		[this](std::vector<ecsql::PreparedSQL>& sqls) {
			auto get_worlds = sqls[0];
			auto update_time_accumulator = sqls[1];
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
				b2WorldId world_id = world_map.at(world_entity_id);
				time_accumulator += delta;
				while (time_accumulator >= timestep) {
					b2World_Step(world_id, timestep, substep_count);
					time_accumulator -= timestep;
				}
				update_time_accumulator(world_entity_id, time_accumulator);
			}
		},
	});
}
