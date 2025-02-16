#include "physics_body.hpp"
#include "physics_shape.hpp"
#include "../ecsql/system.hpp"

std::unordered_map<ecsql::EntityID, b2ShapeId> shape_map;
std::vector<ecsql::EntityID> pending_create_shape;

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
}
