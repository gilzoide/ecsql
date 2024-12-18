#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

inline ecsql::Component BakeRandomScreenPositionComponent {
	"BakeRandomScreenPosition",
	{
		"paddingLeft DEFAULT 0",
		"paddingRight DEFAULT 0",
		"paddingTop DEFAULT 0",
		"paddingBottom DEFAULT 0",
	},
};

void register_bake_random_screen_position_system(ecsql::World& world);
