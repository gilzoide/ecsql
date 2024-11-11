#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/ecsql.hpp"

inline ecsql::RawComponent PositionComponent {
	"Position",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"z DEFAULT 0",
	}
};
inline ecsql::RawComponent RotationComponent {
	"Rotation",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"z DEFAULT 0",
	}
};
inline ecsql::RawComponent ScaleComponent {
	"Scale",
	{
		"x DEFAULT 1",
		"y DEFAULT 1",
		"z DEFAULT 1",
	}
};
inline ecsql::RawComponent PivotComponent {
	"Pivot",
	{
		"x DEFAULT 0.5",
		"y DEFAULT 0.5",
		"z DEFAULT 0.5",
	}
};

inline ecsql::RawComponent ColorComponent {
	"Color",
	{
		"r DEFAULT 255",
		"g DEFAULT 255",
		"b DEFAULT 255",
		"a DEFAULT 255",
	}
};

inline ecsql::RawComponent RectangleComponent {
	"Rectangle",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"width DEFAULT 1",
		"height DEFAULT 1",
	}
};

inline ecsql::RawComponent TextComponent {
	"Text",
	{
		"text TEXT",
		"size DEFAULT 12",
	}
};

void register_draw_systems(ecsql::Ecsql& world);
