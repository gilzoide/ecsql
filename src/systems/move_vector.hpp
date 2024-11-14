#pragma once

#include <raylib.h>

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

inline ecsql::Component<Vector2> MoveVector { "MoveVector" };

void register_move_vector(ecsql::World& world);
