#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

struct BodyUserData {
	ecsql::EntityID entity_id;
};

extern std::unordered_map<ecsql::EntityID, b2BodyId> body_map;

void register_physics_body(ecsql::World& world);
