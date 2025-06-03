#include "line_strip_flyweight.hpp"

ComponentFlyweight<LineStrip> LineStripFlyweight {
	[](const std::string& filename) {
		return LineStrip::parse(filename.c_str());
	},
	std::string_view("PointStrip"),
};
