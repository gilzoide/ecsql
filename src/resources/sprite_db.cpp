#include "sprite_db.hpp"

#include "../assetio.hpp"
#include "../xml_utils.hpp"

#include <physfs_streambuf.hpp>

void SpriteDb::index_path(const std::filesystem::path& texture_root_path, bool recursive) {
	assetio::foreach_file(texture_root_path, [this](const std::filesystem::path& file) {
		std::filesystem::path extension = file.extension();
		if (extension == ".png" || extension == ".basis" || extension == ".ktx2") {
			index_sprite(file.filename(), file);
		}
		else if (extension == ".xml") {
			std::filesystem::path image_path = sprite_to_atlas_map[file.stem()];

			physfs_streambuf stream(file.c_str(), std::ios::in);
			std::istream is(&stream);
			std::string word;
			while (is >> word) {
				if (word.starts_with("name=")) {
					std::filesystem::path sprite_name(extract_xml_value(word));
					index_sprite(sprite_name, image_path);
				}
			}
		}
	}, recursive);
}

void SpriteDb::clear() {
	sprite_to_atlas_map.clear();
}

std::string SpriteDb::get_atlas(const std::string& texture_name) const {
	auto it = sprite_to_atlas_map.find(texture_name);
	if (it != sprite_to_atlas_map.end()) {
		return it->second;
	}
	else {
#if defined(DEBUG) && !defined(NDEBUG)
		if (!texture_name.empty()) {
			std::cerr << "FIXME: accessing unknown sprite " << texture_name << std::endl;
		}
#endif
		return "";
	}
}

SpriteDb& SpriteDb::get_instance() {
	static SpriteDb instance;
	return instance;
}

void SpriteDb::index_sprite(const std::string& sprite_name, const std::string& atlas_path) {
#if defined(DEBUG) && !defined(NDEBUG)
	if (sprite_to_atlas_map.contains(sprite_name)) {
		std::cerr << "FIXME: duplicated sprite " << sprite_name << ". Previously defined in " << sprite_to_atlas_map[sprite_name] << " and redefined in " << atlas_path << std::endl;
	}
#endif
	sprite_to_atlas_map[std::filesystem::path(sprite_name).replace_extension()] = atlas_path;
}
