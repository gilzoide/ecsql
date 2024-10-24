#include "image_flyweight.hpp"
#include "texture_flyweight.hpp"

ComponentFlyweight<Texture2D> TextureFlyweight {
    [](const std::string& key) {
		auto image = ImageFlyweight.get(key);
        return LoadTextureFromImage(image);
    },
    UnloadTexture,
};
