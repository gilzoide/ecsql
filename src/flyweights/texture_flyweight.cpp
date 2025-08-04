#include <raylib.h>

#include "texture_flyweight.hpp"
#include "../ecsql/additional_sql.hpp"

ComponentFlyweight<TextureAtlas> TextureFlyweight {
	[](const std::string& key) {
		return TextureAtlas::load(key);
	},
	[](TextureAtlas& atlas){
		atlas.unload();
	},
	"Texture",
};

ecsql::AdditionalSQL SubtextureSql {
	R"(
		ALTER TABLE Texture
		ADD COLUMN subtexture
	)"
};
