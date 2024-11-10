#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::RawComponent BakePositionComponent {
	"BakePosition",
	{
		"parent_id INTEGER REFERENCES entity(id)",
		"x DEFAULT 0",
		"y DEFAULT 0",
		"z DEFAULT 0",
	},
};

void register_bake_position_system(ecsql::Ecsql& world);
