#include "texture_flyweight.hpp"

ComponentFlyweight<Texture2D> TextureFlyweight {
    [](const std::string& key) {
        return LoadTexture(key.c_str());
    },
    UnloadTexture,
};
