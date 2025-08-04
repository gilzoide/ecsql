#pragma once

#include <string>
#include <unordered_map>

#include <raylib.h>

struct TextureAtlas {
	Texture2D texture;
	std::unordered_map<std::string, Rectangle> subtextures;

	Rectangle subtexture_rect(const std::string& name) const;

	void unload();
	static TextureAtlas load(const std::string& path);
};
