#pragma once

#include <span>
#include <vector>

#include <raylib.h>

class LineStrip {
public:
	LineStrip();
	LineStrip(const std::vector<Vector2>& points);
	LineStrip(std::vector<Vector2>&& points);

	std::span<Vector2> points();
	std::span<const Vector2> points() const;
	std::span<Vector2> looped_points();
	std::span<const Vector2> looped_points() const;

	static LineStrip parse(const char *filename);

private:
	std::vector<Vector2> looped_point_vector;
};
