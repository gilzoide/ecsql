#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Tag MoveOnArrows { "MoveOnArrows" };
inline ecsql::RawComponent LinearSpeed {
	"LinearSpeed",
	{
		"value DEFAULT 1"
	},
};

void register_move_on_arrows(ecsql::Ecsql& world);
