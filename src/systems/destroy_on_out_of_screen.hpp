#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

inline ecsql::Tag DestroyOnOutOfScreen = { "DestroyOnOutOfScreen" };

void register_destroy_on_out_of_screen(ecsql::World& world);
