#pragma once

#include <string>

#include <raylib.h>

struct Sprite {
	std::string name;
	Texture2D texture;
	Rectangle source_rect;
};
