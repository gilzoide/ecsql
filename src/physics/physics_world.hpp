#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

extern std::unordered_map<ecsql::EntityID, b2WorldId> world_map;

void register_physics_world(ecsql::World& world);
