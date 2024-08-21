#pragma once

#include <raylib.h>

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Component<Color> ColorComponent;
inline ecsql::Component<Rectangle> RectangleComponent;
inline ecsql::Component<float> Rotation2DComponent { "Rotation2D" };
