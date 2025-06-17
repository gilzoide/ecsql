#pragma once

#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

void set_entity_id(b2BodyDef& body_def, ecsql::EntityID entity_id);
void set_entity_id(b2BodyId body_id, ecsql::EntityID entity_id);
ecsql::EntityID get_entity_id(b2BodyId body_id);

extern std::unordered_map<ecsql::EntityID, b2BodyId> body_map;

void register_physics_body(ecsql::World& world);
