#include "model_flyweight.hpp"

ComponentFlyweight<Model> ModelFlyweight {
	[](const std::string& key) {
		return LoadModel(key.c_str());
	},
	UnloadModel,
};
