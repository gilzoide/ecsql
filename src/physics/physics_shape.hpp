#pragma once

#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

struct ShapeUserData {
	ecsql::EntityID entity_id;

	static ShapeUserData *from(b2ShapeId body_id);
};

void register_physics_shape(ecsql::World& world);
