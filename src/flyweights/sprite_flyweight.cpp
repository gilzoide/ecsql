#include "sprite_flyweight.hpp"

#include "texture_flyweight.hpp"
#include "../resources/sprite_db.hpp"

static std::string get_atlas_path(const std::string& sprite_name) {
	return SpriteDb::get_instance().get_atlas(sprite_name);
}

ComponentFlyweight<Sprite> SpriteFlyweight {
	[](const std::string& sprite_name) {
		std::string atlas_path = get_atlas_path(sprite_name);
		TextureAtlas& atlas = TextureFlyweight.acquire(atlas_path);
		return Sprite {
			sprite_name,
			atlas.texture,
			atlas.subtexture_rect(sprite_name),
		};
	},
	[](Sprite& sprite) {
		std::string atlas_path = get_atlas_path(sprite.name);
		TextureFlyweight.release(atlas_path);
	},
};
