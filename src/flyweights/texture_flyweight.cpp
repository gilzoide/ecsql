#include "texture_flyweight.hpp"

TextureFlyweight texture_flyweight {
    [](const std::string& key) {
        return LoadTexture(key.c_str());
    },
    UnloadTexture,
};
