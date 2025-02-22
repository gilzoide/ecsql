#include <raylib.h>

#include "physics_body.hpp"
#include "physics_shape.hpp"
#include "../ecsql/system.hpp"

std::unordered_map<ecsql::EntityID, b2ShapeId> shape_map;
std::vector<ecsql::EntityID> pending_create_shape;

struct b2Box {
	b2Vec2 half_size;
	b2Vec2 center;
	float rotation;
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

ecsql::Component CapsuleComponent {
	"Capsule",
	{
		"x1 NOT NULL DEFAULT 0",
		"y1 NOT NULL DEFAULT 0.5",
		"x2 NOT NULL DEFAULT 0",
		"y2 NOT NULL DEFAULT -0.5",
		"radius NOT NULL",
	}
};

ecsql::Component BoxComponent {
	"Box",
	{
		"half_width DEFAULT 0.5",
		"half_height DEFAULT 0.5",
		"x",
		"y",
		"rotation",
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
						b2DestroyShape(it->second, true);
					}
					shape_map.erase(it);
				}
				break;
			}
		}
	}
};

void register_physics_shape(ecsql::World& world) {
	world.register_system({
		"physics.CreateShape",
		R"(
			SELECT
				-- shape
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
				Circle.x, Circle.y, Circle.radius,
				-- capsule
				Capsule.x1, Capsule.y1, Capsule.x2, Capsule.y2, Capsule.radius,
				-- rectangle
				Box.half_width, Box.half_height, Box.x, Box.y, Box.rotation
			FROM Shape
				LEFT JOIN Circle USING(entity_id)
				LEFT JOIN Capsule USING(entity_id)
				LEFT JOIN Box USING(entity_id)
			WHERE entity_id = ?
		)",
		[](ecsql::PreparedSQL& select_shape) {
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
					circle,
					capsule,
					box
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
					std::optional<b2Circle>,
					std::optional<b2Capsule>,
					std::optional<b2Box>
				>();

				if (!circle && !capsule && !box) {
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

				b2ShapeId shape_id = b2_nullShapeId;
				if (circle) {
					shape_id = b2CreateCircleShape(body_id, &shapedef, &*circle);
				}
				if (capsule) {
					shape_id = b2CreateCapsuleShape(body_id, &shapedef, &*capsule);
				}
				if (box) {
					b2Polygon polygon = b2MakeOffsetBox(box->half_size.x, box->half_size.y, box->center, b2MakeRot(box->rotation * DEG2RAD));
					shape_id = b2CreatePolygonShape(body_id, &shapedef, &polygon);
				}

				if (b2Shape_IsValid(shape_id)) {
					shape_map[shape_entity_id] = shape_id;
				}
			}
			pending_create_shape.clear();
		},
	});
}
