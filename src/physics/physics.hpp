#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

class Physics {
public:
	Physics(ecsql::World& world);
	~Physics();

private:
	b2WorldId physics_world;
	ecsql::World& ecs_world;
};
