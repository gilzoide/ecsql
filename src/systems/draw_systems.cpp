#include <string_view>

#include <raylib.h>

#include "draw_systems.hpp"
#include "../ecsql/system.hpp"
#include "../components/texture_reference.hpp"
#include "../flyweights/texture_flyweight.hpp"

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
            SELECT path, x, y, width, height, Rotation2D.value, r, g, b, a
            FROM TextureReference
            INNER JOIN Rectangle USING(entity_id)
            LEFT JOIN Rotation2D USING(entity_id)
            LEFT JOIN Color USING(entity_id)
        )",
    });
}
