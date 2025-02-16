#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

class Physics {
public:
	Physics(ecsql::World& world);

private:
	ecsql::World& ecs_world;
};
