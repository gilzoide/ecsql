#include <iostream>
#include <raylib.h>

#include "rotate_on_hover.hpp"
#include "../ecsql/system.hpp"

void register_rotate_on_hover_systems(ecsql::Ecsql& world) {
    world.register_system({
        "rotate_on_hover",
        [](auto& select_rect, auto& update_rotation) {
            Vector2 mouse_position = GetMousePosition();
            for (ecsql::SQLRow row : select_rect(mouse_position.x, mouse_position.y)) {
                auto id = row.get<Entity>(0);
                update_rotation(id).step_single();
            }
        },
        R"(
            SELECT entity_id
            FROM RotateOnHover NATURAL JOIN Rectangle NATURAL JOIN Rotation2D
            WHERE x <= ?1 AND y <= ?2 AND x+width >= ?1 AND y+height >= ?2
        )",
        "UPDATE Rotation2D SET value = value + 1 WHERE entity_id = ?"
    });
}
