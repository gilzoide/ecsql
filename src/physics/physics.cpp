#include <flyweight.hpp>
#include <box2d/box2d.h>

#include "physics.hpp"
#include "../ecsql/system.hpp"
#include "../ecsql/hook_system.hpp"

std::unordered_map<ecsql::EntityID, b2BodyId> body_map;
std::vector<ecsql::EntityID> pending_create_body;

ecsql::Component BodyComponent {
	"Body",
	{
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
	b2WorldDef world_def = b2DefaultWorldDef();
	physics_world = b2CreateWorld(&world_def);

	world.register_system({
		"physics.CreateBody",
		R"(
			SELECT
				-- entity
				name,
				-- body
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
		)",
		[this](ecsql::PreparedSQL select_body) {
			for (auto entity_id : pending_create_body) {
				auto [
					name,
					type,
					linear_damping, angular_damping, gravity_scale, sleep_threshold,
					enable_sleep, is_awake, fixed_rotation, is_bullet, is_enabled, allow_fast_rotation,
					position,
					rotation_angle
				] = select_body(entity_id).get<
					std::optional<const char *>,
					std::optional<std::string_view>,
					std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
					std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>,
					std::optional<b2Vec2>,
					std::optional<float>
				>();
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
				b2BodyId body_id = b2CreateBody(physics_world, &bodydef);
				body_map[entity_id] = body_id;
			}
			pending_create_body.clear();
		},
	});
}

Physics::~Physics() {
	b2DestroyWorld(physics_world);
}
