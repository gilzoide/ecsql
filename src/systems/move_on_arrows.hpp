#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

inline ecsql::Tag MoveOnArrows { "MoveOnArrows" };
inline ecsql::RawComponent LinearSpeed {
	"LinearSpeed",
	{
		"value DEFAULT 1"
	},
};

void register_move_on_arrows(ecsql::World& world);
