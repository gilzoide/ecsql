#include <iostream>

#include <cdedent.hpp>
#include <raylib.h>

#include "rotate_on_hover.hpp"
#include "../ecsql/system.hpp"

void register_rotate_on_hover_systems(ecsql::Ecsql& world) {
    world.register_system({
        "rotate_on_hover",
        [](auto& select_rect, auto& update_rotation) {
            Vector2 mouse_position = GetMousePosition();
            for (ecsql::SQLRow row : select_rect(mouse_position.x, mouse_position.y)) {
                update_rotation(row.get<Entity>());
            }
        },
        R"(
            SELECT entity_id
            FROM RotateOnHover
			JOIN Rectangle USING(entity_id)
			JOIN Rotation USING(entity_id)
            WHERE Rectangle.x <= ?1 AND Rectangle.y <= ?2 AND Rectangle.x + width >= ?1 AND Rectangle.y + height >= ?2
        )"_dedent,
        "UPDATE Rotation SET z = z + 1 WHERE entity_id = ?"
    });
}
