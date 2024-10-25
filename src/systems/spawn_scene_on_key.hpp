#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::RawComponent SpawnerTag {
	"SpawnOnKey",
	{
		"cooldown DEFAULT 0",
		"last_spawn_time",
	},
};

void register_spawn_scene_on_key(ecsql::Ecsql& world);
