#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::RawComponent SpawnerTag {
	"Spawner",
	{
		// Spawned object properties
		"direction_x DEFAULT 0",
		"direction_y DEFAULT -1",
		"texture_path",
		// Spawn cooldown
		"cooldown DEFAULT 0",
		"last_spawn_time",
	},
};

void register_spawn_moving_object(ecsql::Ecsql& world);
