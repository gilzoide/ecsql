#include "line_strip.hpp"

#include <raymath.h>
#include <physfs_funopen.h>

LineStrip::LineStrip()
	: looped_point_vector()
{
}

LineStrip::LineStrip(const std::vector<Vector2>& points)
	: looped_point_vector(points)
{
	if (looped_point_vector.front() != looped_point_vector.back()) {
		looped_point_vector.push_back(looped_point_vector.front());
	}
}

LineStrip::LineStrip(std::vector<Vector2>&& points)
	: looped_point_vector(std::move(points))
{
	if (looped_point_vector.front() != looped_point_vector.back()) {
		looped_point_vector.push_back(looped_point_vector.front());
	}
}

std::span<Vector2> LineStrip::points() {
	return std::span(looped_point_vector.data(), looped_point_vector.size() - 1);
}

std::span<const Vector2> LineStrip::points() const {
	return std::span(looped_point_vector.data(), looped_point_vector.size() - 1);
}

std::span<Vector2> LineStrip::looped_points() {
	return std::span(looped_point_vector.data(), looped_point_vector.size());
}

std::span<const Vector2> LineStrip::looped_points() const {
	return std::span(looped_point_vector.data(), looped_point_vector.size());
}

LineStrip LineStrip::parse(const char *filename) {
	std::vector<Vector2> points;
	Vector2 point;
	FILE *stream = PHYSFS_funopenRead(filename);
	while (fscanf(stream, " %f, %f", &point.x, &point.y) == 2) {
		points.push_back(point);
	}
	fclose(stream);
	return LineStrip(std::move(points));
}
