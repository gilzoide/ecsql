#pragma once

#include <raylib.h>

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Component<Vector2> MoveVector { "MoveVector" };

void register_move_vector(ecsql::Ecsql& world);
