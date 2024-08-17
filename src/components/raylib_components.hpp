#pragma once

#include <raylib.h>

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Component<Color> ColorComponent;
inline ecsql::Component<Rectangle> RectangleComponent;

void register_raylib_components(ecsql::Ecsql& world);
