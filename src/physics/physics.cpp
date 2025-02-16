#include <cdedent.hpp>
#include <box2d/box2d.h>

#include "physics.hpp"
#include "../ecsql/system.hpp"
#include "../ecsql/hook_system.hpp"

std::unordered_map<ecsql::EntityID, b2WorldId> world_map;
std::unordered_map<ecsql::EntityID, b2BodyId> body_map;
std::unordered_map<ecsql::EntityID, b2ShapeId> shape_map;
std::vector<ecsql::EntityID> pending_create_body;
std::vector<ecsql::EntityID> pending_create_shape;

struct BodyUserData {
	ecsql::EntityID entity_id;
};

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

ecsql::Component ShapeComponent {
	"Shape",
	{
		"friction",
		"restitution",
		"rolling_resistance",
		"tangent_speed",
		"material",
		"density",
		// "filter", TODO
		"is_sensor",
		"enable_contact_events",
		"enable_hit_events",
		"enable_pre_solve_events",
		"invoke_contact_creation",
		"update_body_mass",
	}
};

ecsql::Component CircleComponent {
	"Circle",
	{
		"x NOT NULL DEFAULT 0",
		"y NOT NULL DEFAULT 0",
		"radius NOT NULL",
	}
};

ecsql::HookSystem ShapeHookSystem {
	ShapeComponent,
	[](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
		switch (hook) {
			case ecsql::HookType::OnInsert:
				pending_create_shape.push_back(new_row.get<ecsql::EntityID>(0));
				break;

			case ecsql::HookType::OnUpdate:
				break;

			case ecsql::HookType::OnDelete: {
				auto it = shape_map.find(old_row.get<ecsql::EntityID>(0));
				if (it != shape_map.end()) {
					if (b2Shape_IsValid(it->second)) {
						delete reinterpret_cast<BodyUserData *>(b2Shape_GetUserData(it->second));
						b2DestroyShape(it->second, true);
					}
					shape_map.erase(it);
				}
				break;
			}
		}
	}
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
					bodydef.rotation = b2MakeRot(*rotation_angle);
				}
				b2BodyId body_id = b2CreateBody(world_id, &bodydef);
				body_map[entity_id] = body_id;
			}
			pending_create_body.clear();
		},
	});
	world.register_system({
		"physics.CreateShape",
		R"(
			SELECT
				friction,
				restitution,
				rolling_resistance,
				tangent_speed,
				material,
				density,
				is_sensor,
				enable_contact_events,
				enable_hit_events,
				enable_pre_solve_events,
				invoke_contact_creation,
				update_body_mass,
				-- circle
				Circle.x, Circle.y, Circle.radius
			FROM Shape
				LEFT JOIN Circle USING(entity_id)
			WHERE entity_id = ?
		)",
		[this](ecsql::PreparedSQL& select_shape) {
			for (auto shape_entity_id : pending_create_shape) {
				auto body_it = body_map.find(shape_entity_id);
				if (body_it == body_map.end()) {
					continue;
				}

				b2BodyId body_id = body_it->second;
				if (!b2Body_IsValid(body_it->second)) {
					continue;
				}

				auto [
					friction,
					restitution,
					rolling_resistance,
					tangent_speed,
					material,
					density,
					is_sensor,
					enable_contact_events,
					enable_hit_events,
					enable_pre_solve_events,
					invoke_contact_creation,
					update_body_mass,
					circle
				] = select_shape(shape_entity_id).get<
					std::optional<float>,
					std::optional<float>,
					std::optional<float>,
					std::optional<float>,
					std::optional<int>,
					std::optional<float>,
					std::optional<bool>,
					std::optional<bool>,
					std::optional<bool>,
					std::optional<bool>,
					std::optional<bool>,
					std::optional<bool>,
					std::optional<b2Circle>
				>();

				if (!circle) {
					continue;
				}

				b2ShapeDef shapedef = b2DefaultShapeDef();
				if (friction) {
					shapedef.friction = *friction;
				}
				if (restitution) {
					shapedef.restitution = *restitution;
				}
				if (rolling_resistance) {
					shapedef.rollingResistance = *rolling_resistance;
				}
				if (tangent_speed) {
					shapedef.tangentSpeed = *tangent_speed;
				}
				if (material) {
					shapedef.material = *material;
				}
				if (density) {
					shapedef.density = *density;
				}
				if (is_sensor) {
					shapedef.isSensor = *is_sensor;
				}
				if (enable_contact_events) {
					shapedef.enableContactEvents = *enable_contact_events;
				}
				if (enable_hit_events) {
					shapedef.enableHitEvents = *enable_hit_events;
				}
				if (enable_pre_solve_events) {
					shapedef.enablePreSolveEvents = *enable_pre_solve_events;
				}
				if (invoke_contact_creation) {
					shapedef.invokeContactCreation = *invoke_contact_creation;
				}
				if (update_body_mass) {
					shapedef.updateBodyMass = *update_body_mass;
				}

				b2ShapeId shape_id;
				if (circle) {
					shape_id = b2CreateCircleShape(body_id, &shapedef, &circle.value());
				}

				if (b2Shape_IsValid(shape_id)) {
					shape_map[shape_entity_id] = shape_id;
				}
			}
			pending_create_shape.clear();
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
			R"(
				REPLACE INTO Position(entity_id, x, y)
				VALUES(?, ?, ?)
			)"_dedent,
			R"(
				REPLACE INTO Rotation(entity_id, z)
				VALUES(?, ?)
			)"_dedent,
		},
		[this](std::vector<ecsql::PreparedSQL>& sqls) {
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
				// Simulate world
				b2WorldId world_id = world_map.at(world_entity_id);
				time_accumulator += delta;
				while (time_accumulator >= timestep) {
					b2World_Step(world_id, timestep, substep_count);
					time_accumulator -= timestep;
				}
				update_time_accumulator(world_entity_id, time_accumulator);

				// Update body data
				b2BodyEvents body_events = b2World_GetBodyEvents(world_id);
				for (auto move_event : std::span<b2BodyMoveEvent>(body_events.moveEvents, body_events.moveCount)) {
					BodyUserData *user_data = reinterpret_cast<BodyUserData *>(b2Body_GetUserData(move_event.bodyId));

					b2Vec2 position = move_event.transform.p;
					update_position(user_data->entity_id, position);

					float rotation = b2Rot_GetAngle(move_event.transform.q);
					update_rotation(user_data->entity_id, rotation);
				}
			}

		},
	});
}
