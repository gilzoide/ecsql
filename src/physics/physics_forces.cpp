#include "box2d/box2d.h"
#include "physics_body.hpp"
#include "physics_forces.hpp"
#include "../ecsql/system.hpp"

void register_physics_forces(ecsql::World& world) {
	world.register_system({
		"physics.ApplyForce",
		{
			R"(
				SELECT
					entity_id,
					x, y,
					point_x, point_y,
					is_local,
					wake
				FROM Force
			)",
			"DELETE FROM Force",
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto select_forces = sqls[0];
			auto delete_forces = sqls[1];
			for (auto row : select_forces()) {
				auto [
					entity_id,
					force,
					point,
					is_local,
					wake
				] = row.get<
					ecsql::EntityID,
					b2Vec2,
					std::optional<b2Vec2>,
					bool,
					bool
				>();

				auto it = body_map.find(entity_id);
				if (it != body_map.end()) {
					if (is_local) {
						b2Rot rotation = b2Body_GetRotation(it->second);
						force = b2RotateVector(rotation, force);
					}

					if (point) {
						b2Body_ApplyForce(it->second, force, *point, wake);
					}
					else {
						b2Body_ApplyForceToCenter(it->second, force, wake);
					}
				}
				else {
					std::cerr << "Trying to apply force to unknown body " << entity_id << std::endl;
				}
			}
			delete_forces();
		},
	});

	world.register_system({
		"physics.ApplyLinearImpulse",
		{
			R"(
				SELECT
					entity_id,
					x, y,
					point_x, point_y,
					is_local,
					wake
				FROM LinearImpulse
			)",
			"DELETE FROM LinearImpulse",
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto select_impulses = sqls[0];
			auto delete_impulses = sqls[1];
			for (auto row : select_impulses()) {
				auto [
					entity_id,
					impulse,
					point,
					is_local,
					wake
				] = row.get<
					ecsql::EntityID,
					b2Vec2,
					std::optional<b2Vec2>,
					bool,
					bool
				>();

				auto it = body_map.find(entity_id);
				if (it != body_map.end()) {
					if (is_local) {
						b2Rot rotation = b2Body_GetRotation(it->second);
						impulse = b2RotateVector(rotation, impulse);
					}

					if (point) {
						b2Body_ApplyLinearImpulse(it->second, impulse, *point, wake);
					}
					else {
						b2Body_ApplyLinearImpulseToCenter(it->second, impulse, wake);
					}
				}
				else {
					std::cerr << "Trying to apply impulse to unknown body " << entity_id << std::endl;
				}
			}
			delete_impulses();
		},
	});

	world.register_system({
		"physics.ApplyTorque",
		{
			R"(
				SELECT
					entity_id,
					angle,
					wake
				FROM Torque
			)",
			"DELETE FROM Torque",
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto select_torque = sqls[0];
			auto delete_torque = sqls[1];
			for (auto row : select_torque()) {
				auto [
					entity_id,
					torque,
					wake
				] = row.get<
					ecsql::EntityID,
					float,
					bool
				>();

				auto it = body_map.find(entity_id);
				if (it != body_map.end()) {
					b2Body_ApplyTorque(it->second, torque, wake);
				}
				else {
					std::cerr << "Trying to apply torque to unknown body " << entity_id << std::endl;
				}
			}
			delete_torque();
		},
	});

	world.register_system({
		"physics.SetAngularVelocity",
		{
			R"(
				SELECT
					entity_id,
					z
				FROM SetAngularVelocity
			)",
			"DELETE FROM SetAngularVelocity",
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto select_pending_updates = sqls[0];
			auto delete_pending = sqls[1];
			for (auto row : select_pending_updates()) {
				auto [
					entity_id,
					angular_velocity
				] = row.get<
					ecsql::EntityID,
					float
				>();

				auto it = body_map.find(entity_id);
				if (it != body_map.end()) {
					b2Body_SetAngularVelocity(it->second, angular_velocity);
				}
				else {
					std::cerr << "Trying to set angular velocity of unknown body " << entity_id << std::endl;
				}
			}
			delete_pending();
		},
	});
}
