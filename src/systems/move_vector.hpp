#pragma once

#include <raylib.h>

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

inline ecsql::Component MoveVector {
	"MoveVector",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
	},
};

void register_move_vector(ecsql::World& world);
