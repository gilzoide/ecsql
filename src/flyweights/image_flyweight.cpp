#include "image_flyweight.hpp"

ComponentFlyweight<Image> ImageFlyweight {
    [](const std::string& key) {
        return LoadImage(key.c_str());
    },
    UnloadImage,
};
