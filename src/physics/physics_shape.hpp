#pragma once

#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

void set_entity_id(b2ShapeDef& shape_def, ecsql::EntityID entity_id);
void set_entity_id(b2ShapeId shape_id, ecsql::EntityID entity_id);
ecsql::EntityID get_entity_id(b2ShapeId shape_id);

void register_physics_shape(ecsql::World& world);
