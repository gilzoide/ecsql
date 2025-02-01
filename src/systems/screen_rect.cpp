#include "screen_rect.hpp"
#include "../ecsql/system.hpp"
#include "raylib.h"

void register_update_screen_rect(ecsql::World& world) {
	world.register_system({
		"UpdateScreenRect",
		R"(
			INSERT OR REPLACE INTO Rectangle(entity_id, width, height)
			SELECT entity_id, width, height
			FROM ScreenRect
				JOIN screen_size
		)",
		[](auto& sql) {
			static bool first_time = true;
			if (first_time || IsWindowResized()) {
				sql();
				first_time = false;
			}
		},
	});
}
