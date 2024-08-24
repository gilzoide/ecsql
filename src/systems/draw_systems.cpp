#include <string_view>

#include <raylib.h>

#include "draw_systems.hpp"
#include "../ecsql/system.hpp"
#include "../components/texture_reference.hpp"

void register_draw_systems(ecsql::Ecsql& world) {
    world.register_system({
        "DrawTextureRect",
        [](auto& sql) {
            for (ecsql::SQLRow row : sql()) {
                auto [texref, rectangle, rotation, color] = row.get<TextureReference, Rectangle, float, std::optional<Color>>(0);
                auto tex = texref.get();
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
                auto [texref, position, rotation, color] = row.get<TextureReference, Vector2, float, std::optional<Color>>(0);
                auto tex = texref.get();
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
}
