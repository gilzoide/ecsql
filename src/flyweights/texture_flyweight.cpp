#include "texture_flyweight.hpp"

Texture2D construct_texture(const std::string& key) {
    return LoadTexture(key.c_str());
}

TextureFlyweight texture_flyweight {
    construct_texture,
    UnloadTexture,
};
