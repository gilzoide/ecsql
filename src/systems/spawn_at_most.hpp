#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

inline ecsql::Component SpawnAtMostComponent {
	"SpawnAtMost",
	{
		"name DEFAULT ''",
		"count DEFAULT 1",
	},
};

void register_spawn_at_most(ecsql::World& world);
