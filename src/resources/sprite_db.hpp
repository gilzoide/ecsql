#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>


class SpriteDb {
public:
	void index_path(const std::filesystem::path& texture_root_path, bool recursive = true);
	void clear();

	std::string get_atlas(const std::string& texture_name) const;

	static SpriteDb& get_instance();

private:
	std::unordered_map<std::string, std::string> sprite_to_atlas_map;

	void index_sprite(const std::string& sprite_name, const std::string& atlas_path);
};
