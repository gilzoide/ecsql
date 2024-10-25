#include "scene_sql_flyweight.hpp"
#include "../ecsql/assetio.hpp"
#include "../ecsql/serialization.hpp"

ComponentFlyweight<std::string> SceneSqlFlyweight {
	[](const std::string& key) {
		std::string scene_source = ecsql::read_asset_text(key.c_str());
		return ecsql::load_scene(scene_source, key);
	},
	std::string_view("SceneSql"),
};
