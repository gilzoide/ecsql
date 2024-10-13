#include "material_flyweight.hpp"

ComponentFlyweight<std::span<Material>> MaterialSetFlyweight {
	[](const std::string& filename) {
		int material_count;
		Material *materials = LoadMaterials(filename.c_str(), &material_count);
		return std::span(materials, material_count);
	},
	[](std::span<Material>& materials) {
		for (auto& material : materials) {
			UnloadMaterial(material);
		}
		RL_FREE(materials.data());
	},
	"MaterialSet",
};
