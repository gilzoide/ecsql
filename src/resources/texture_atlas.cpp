#include "texture_atlas.hpp"

#include <charconv>
#include <filesystem>

#include <rlgl.h>
#include <physfs_streambuf.hpp>
#include <string_view>

#include "../xml_utils.hpp"
#include "../flyweights/image_flyweight.hpp"

static void parse_subtextures_xml(std::unordered_map<std::string, Rectangle>& map, std::istream& xml) {
	std::string word;
	decltype(map.emplace()) last_rect;
	while (xml >> word) {
		if (word.starts_with("name")) {
			std::string_view name = extract_xml_value(word);
			last_rect = map.emplace(std::filesystem::path(name).replace_extension(), Rectangle());
		}
		if (word.starts_with("x")) {
			std::string_view x_str = extract_xml_value(word);
			int x;
			std::from_chars(x_str.begin(), x_str.end(), x);
			last_rect.first->second.x = x;
		}
		if (word.starts_with("y")) {
			std::string_view y_str = extract_xml_value(word);
			int y;
			std::from_chars(y_str.begin(), y_str.end(), y);
			last_rect.first->second.y = y;
		}
		if (word.starts_with("width")) {
			std::string_view width_str = extract_xml_value(word);
			int width;
			std::from_chars(width_str.begin(), width_str.end(), width);
			last_rect.first->second.width = width;
		}
		if (word.starts_with("height")) {
			std::string_view height_str = extract_xml_value(word);
			int height;
			std::from_chars(height_str.begin(), height_str.end(), height);
			last_rect.first->second.height = height;
		}
	}
}

Rectangle TextureAtlas::subtexture_rect(const std::string& name) const {
	auto it = subtextures.find(name);
	if (it != subtextures.end()) {
		return it->second;
	}
	else {
		return Rectangle {
			0, 0,
			(float) texture.width, (float) texture.height,
		};
	}
}

TextureAtlas TextureAtlas::load(const std::string& path) {
	if (path.empty()) {
		TextureAtlas atlas = {
			.texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 },
			.subtextures = {},
		};
		return atlas;
	}

	Texture texture = LoadTextureFromImage(ImageFlyweight.get(path));
	TextureAtlas atlas = { .texture = texture, .subtextures = {} };

	physfs_streambuf file(std::filesystem::path(path).replace_extension("xml").c_str(), std::ios::in);
	std::istream is(&file);
	parse_subtextures_xml(atlas.subtextures, is);
	return std::move(atlas);
}

void TextureAtlas::unload() {
	if (texture.id != rlGetTextureIdDefault()) {
		UnloadTexture(texture);
	}
	subtextures.clear();
}
