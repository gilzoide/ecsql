#if defined(DEBUG) && !defined(NDEBUG)

#include "debug.hpp"

#include <iostream>

#include <tracy/Tracy.hpp>
#include <raylib.h>

#include "ecsql/world.hpp"


static bool paused = false;

bool is_paused() {
	return paused;
}

void run_debug_functionality(ecsql::World& world) {
	ZoneScoped;
	DrawFPS(0, 0);

	for (int fkey = KEY_F1; fkey <= KEY_F10; fkey++) {
		bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
		if (IsKeyPressed(fkey)) {
			const char *world_db_path = TextFormat("ecsql_world-backup%02d.sqlite3", fkey - KEY_F1 + 1);
			const char *save_db_path = TextFormat("save-backup%02d.sqlite3", fkey - KEY_F1 + 1);
			if (is_shift_down) {
				if (world.backup_into(world_db_path)) {
					std::cout << "World backed up into \"" << world_db_path << "\"" << std::endl;
				}
				if (world.backup_into(save_db_path, "save")) {
					std::cout << "Save backed up into \"" << save_db_path << "\"" << std::endl;
				}
			}
			else {
				if (world.restore_from(world_db_path)) {
					std::cout << "World restored from \"" << world_db_path << "\"" << std::endl;
				}
				if (world.restore_from(save_db_path, "save")) {
					std::cout << "Save restored from \"" << save_db_path << "\"" << std::endl;
				}
			}
		}
	}

	if (IsKeyPressed(KEY_P)) {
		paused = !paused;
	}

	if (paused) {
		DrawText("PAUSED", 0, 24, 20, LIME);
	}
}


#endif
