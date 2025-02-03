#include <raylib.h>
#include <rlgl.h>

#include "image_flyweight.hpp"
#include "texture_flyweight.hpp"

ComponentFlyweight<Texture2D> TextureFlyweight {
    [](const std::string& key) {
		if (key.empty()) {
			return Texture2D { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
		}
		else {
			auto image = ImageFlyweight.get(key);
			return LoadTextureFromImage(image);
		}
    },
    [](Texture2D& texture) {
		if (texture.id != rlGetTextureIdDefault()) {
			UnloadTexture(texture);
		}
	},
};
