#pragma once

#include <raylib.h>

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::Component<Color> ColorComponent;
inline ecsql::Component<Vector3> PositionComponent { "Position" };
inline ecsql::Component<Vector3> ScaleComponent { "Scale" };
inline ecsql::Component<Rectangle> RectangleComponent;
inline ecsql::Component<Vector3> PivotComponent { "Pivot" };
inline ecsql::Component<Vector3> RotationComponent { "Rotation" };
