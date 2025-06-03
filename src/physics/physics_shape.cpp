#include <raylib.h>

#include "physics_body.hpp"
#include "physics_shape.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/line_strip_flyweight.hpp"

ShapeUserData *ShapeUserData::from(b2ShapeId shape_id) {
	return (ShapeUserData *) b2Shape_GetUserData(shape_id);
}

std::vector<std::pair<ecsql::EntityID, ecsql::EntityID>> pending_create_shape;

struct b2Box {
	b2Vec2 half_size;
	b2Vec2 center;
	float rotation;
};

ecsql::Component ShapeComponent {
	"Shape",
	{
		"body INTEGER REFERENCES Body(entity_id)",  // Body entity id. If NULL, defaults to Shape.entity_id
		"friction",
		"restitution",
		"rolling_resistance",
		"tangent_speed",
		"material_id",
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
				pending_create_shape.push_back(new_row.get<ecsql::EntityID, ecsql::EntityID>(0));
				break;

			case ecsql::HookType::OnUpdate:
				break;

			case ecsql::HookType::OnDelete:
				break;
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
				material_id,
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
				Box.half_width, Box.half_height, Box.x, Box.y, Box.rotation,
				-- convex hull
				PointStrip.path
			FROM Shape
				LEFT JOIN Circle USING(entity_id)
				LEFT JOIN Capsule USING(entity_id)
				LEFT JOIN Box USING(entity_id)
				LEFT JOIN PointStrip USING(entity_id)
			WHERE entity_id = ?
		)",
		[](ecsql::PreparedSQL& select_shape) {
			for (auto [shape_entity_id, body_entity_id] : pending_create_shape) {
				auto body_it = body_map.find(body_entity_id ?: shape_entity_id);
				if (body_it == body_map.end()) {
					continue;
				}

				b2BodyId body_id = body_it->second;
				if (!b2Body_IsValid(body_id)) {
					continue;
				}

				auto [
					friction,
					restitution,
					rolling_resistance,
					tangent_speed,
					material_id,
					density,
					is_sensor,
					enable_contact_events,
					enable_hit_events,
					enable_pre_solve_events,
					invoke_contact_creation,
					update_body_mass,
					circle,
					capsule,
					box,
					line_strip_path
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
					std::optional<b2Box>,
					std::optional<std::string_view>
				>();

				if (!circle && !capsule && !box && !line_strip_path) {
					continue;
				}

				std::unique_ptr<ShapeUserData> userData = std::make_unique<ShapeUserData>(shape_entity_id);

				b2ShapeDef shapedef = b2DefaultShapeDef();
				shapedef.userData = userData.get();
				if (friction) {
					shapedef.material.friction = *friction;
				}
				if (restitution) {
					shapedef.material.restitution = *restitution;
				}
				if (rolling_resistance) {
					shapedef.material.rollingResistance = *rolling_resistance;
				}
				if (tangent_speed) {
					shapedef.material.tangentSpeed = *tangent_speed;
				}
				if (material_id) {
					shapedef.material.userMaterialId = *material_id;
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
				if (line_strip_path) {
					auto line_strip = LineStripFlyweight.get(*line_strip_path);
					auto points = line_strip.value.points();
					b2Hull hull = b2ComputeHull((const b2Vec2 *) points.data(), points.size());
					b2Polygon polygon = b2MakePolygon(&hull, 1);
					shape_id = b2CreatePolygonShape(body_id, &shapedef, &polygon);
				}

				if (b2Shape_IsValid(shape_id)) {
					std::ignore = userData.release();
				}
			}
			pending_create_shape.clear();
		},
	});
}
