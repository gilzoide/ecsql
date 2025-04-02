#include "physics.hpp"
#include "physics_body.hpp"
#include "physics_forces.hpp"
#include "physics_shape.hpp"
#include "physics_world.hpp"

Physics::Physics(ecsql::World& world)
	: ecs_world(world)
{
	register_physics_body(world);
	register_physics_shape(world);
	register_physics_forces(world);
	register_physics_world(world);
}
