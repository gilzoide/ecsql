#include "image_flyweight.hpp"

#include <raylib_basis_universal.h>

ComponentFlyweight<Image> ImageFlyweight {
	[](const std::string& key) {
		if (key.ends_with(".basis") || key.ends_with(".ktx2")) {
			return LoadBasisUniversalImage(key.c_str());
		}
		else {
			return LoadImage(key.c_str());
		}
	},
	UnloadImage,
};
