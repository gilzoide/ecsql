#include <string_view>

#include <raylib.h>

#include "draw_systems.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/model_flyweight.hpp"
#include "../flyweights/texture_flyweight.hpp"

void register_draw_systems(ecsql::Ecsql& world) {
    world.register_system({
        "DrawTextureRect",
        [](auto& sql) {
            for (ecsql::SQLRow row : sql()) {
                auto [tex_path, rectangle, rotation, color] = row.get<std::string_view, Rectangle, float, std::optional<Color>>();
                auto tex = TextureFlyweight.get(tex_path);
                Rectangle source {
                    0, 0,
                    (float) tex.value.width, (float) tex.value.height,
                };
                Vector2 center { rectangle.width * 0.5f, rectangle.height * 0.5f };
                rectangle.x += center.x;
                rectangle.y += center.y;
                DrawTexturePro(tex, source, rectangle, center, rotation, color.value_or(WHITE));
            }
        },
        R"(
            SELECT path, Rectangle.x, Rectangle.y, width, height, Rotation.z, r, g, b, a
            FROM TextureReference
            JOIN Rectangle USING(entity_id)
            LEFT JOIN Rotation USING(entity_id)
            LEFT JOIN Color USING(entity_id)
        )",
    });
    world.register_system({
        "DrawTexture",
        [](auto& sql) {
            for (ecsql::SQLRow row : sql()) {
                auto [tex_path, position, rotation, color] = row.get<std::string_view, Vector2, float, std::optional<Color>>();
                auto tex = TextureFlyweight.get(tex_path);
                Rectangle source {
                    0, 0,
                    (float) tex.value.width, (float) tex.value.height,
                };
                Vector2 center { source.width * 0.5f, source.height * 0.5f };
				Rectangle dest {
					position.x + center.x,
					position.y + center.y,
					source.width,
					source.height,
				};
                DrawTexturePro(tex, source, dest, center, rotation, color.value_or(WHITE));
            }
        },
        R"(
            SELECT path, Position.x, Position.y, Rotation.z, r, g, b, a
            FROM TextureReference
            JOIN Position USING(entity_id)
            LEFT JOIN Rotation USING(entity_id)
            LEFT JOIN Color USING(entity_id)
        )",
    });
	world.register_system({
		"DrawModel",
		[](auto& sql) {
			Camera3D camera {
				{ 10, 10, 10 },
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				45,
				CAMERA_PERSPECTIVE,
			};
			BeginMode3D(camera);
			for (ecsql::SQLRow row : sql()) {
				auto [model_path, position, color] = row.get<std::string_view, Vector3, std::optional<Color>>();
                auto model = ModelFlyweight.get(model_path);
                DrawModel(model, position, 1, color.value_or(WHITE));
			}
			EndMode3D();
		},
		R"(
			SELECT path, Position.x, Position.y, Position.z, r, g, b, a
            FROM ModelReference
            JOIN Position USING(entity_id)
            LEFT JOIN Color USING(entity_id)
		)",
	});
}
