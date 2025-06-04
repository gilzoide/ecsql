#include "line_strip.hpp"

#include <istream>

#include <physfs_streambuf.hpp>
#include <raymath.h>

LineStrip::LineStrip()
	: looped_point_vector()
{
}

LineStrip::LineStrip(const std::vector<Vector2>& points)
	: looped_point_vector(points)
{
	if (!looped_point_vector.empty() && looped_point_vector.front() != looped_point_vector.back()) {
		looped_point_vector.push_back(looped_point_vector.front());
	}
}

LineStrip::LineStrip(std::vector<Vector2>&& points)
	: looped_point_vector(std::move(points))
{
	if (!looped_point_vector.empty() && looped_point_vector.front() != looped_point_vector.back()) {
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

LineStrip LineStrip::scaled(Vector2 scale) const {
	std::vector<Vector2> copied_points = looped_point_vector;
	for (Vector2& point : copied_points) {
		point *= scale;
	}
	return LineStrip(std::move(copied_points));
}

LineStrip LineStrip::parse(const char *filename) {
	std::vector<Vector2> points;
	physfs_streambuf buf(filename, std::ios::in);
	std::istream is(&buf);
	while (true) {
		Vector2 point;
		is >> point.x;
		is.ignore(1, ',');
		is >> point.y;
		if (is.fail() || is.eof()) {
			break;
		}
		points.push_back(point);
	}
	return LineStrip(std::move(points));
}
