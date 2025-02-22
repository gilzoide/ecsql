#include <box2d/box2d.h>
#include <cdedent.hpp>
#include <raylib.h>

#include "physics_body.hpp"
#include "physics_world.hpp"
#include "../ecsql/system.hpp"

std::unordered_map<ecsql::EntityID, b2BodyId> body_map;
std::vector<ecsql::EntityID> pending_create_body;

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
						delete reinterpret_cast<BodyUserData *>(b2Body_GetUserData(it->second));
						b2DestroyBody(it->second);
					}
					body_map.erase(it);
				}
				break;
			}
		}
	},
};

BodyUserData *BodyUserData::from(b2BodyId body_id) {
	return (BodyUserData *) b2Body_GetUserData(body_id);
}

void register_physics_body(ecsql::World& world) {
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
					Rotation.z,
					-- linear velocity
					LinearVelocity.x, LinearVelocity.y,
					-- angular velocity
					AngularVelocity.z
				FROM Body
					JOIN entity ON Body.entity_id = entity.id
					LEFT JOIN Position USING(entity_id)
					LEFT JOIN Rotation USING(entity_id)
					LEFT JOIN LinearVelocity USING(entity_id)
					LEFT JOIN AngularVelocity USING(entity_id)
				WHERE entity_id = ?
			)"_dedent,
			R"(
				SELECT entity_id
				FROM World
				LIMIT 1
			)"_dedent,
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
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
					rotation_angle,
					linear_velocity,
					angular_velocity
				] = select_body(entity_id).get<
					std::optional<const char *>,
					std::optional<ecsql::EntityID>,
					std::optional<std::string_view>,
					std::optional<float>, std::optional<float>, std::optional<float>, std::optional<float>,
					std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>, std::optional<bool>,
					std::optional<b2Vec2>,
					std::optional<float>,
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
				bodydef.userData = new BodyUserData {
					.entity_id = entity_id,
				};
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
					bodydef.rotation = b2MakeRot(*rotation_angle * DEG2RAD);
				}
				if (linear_velocity) {
					bodydef.linearVelocity = *linear_velocity;
				}
				if (angular_velocity) {
					bodydef.angularVelocity = *angular_velocity * DEG2RAD;
				}
				b2BodyId body_id = b2CreateBody(world_id, &bodydef);
				body_map[entity_id] = body_id;
			}
			pending_create_body.clear();
		},
	});
}
