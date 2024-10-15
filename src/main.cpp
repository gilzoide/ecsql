#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <idbvfs.h>
#endif
#include <raylib.h>
#include <tracy/Tracy.hpp>

#include "memory.hpp"
#include "ecsql/assetio.hpp"
#include "ecsql/component.hpp"
#include "ecsql/ecsql.hpp"
#include "ecsql/hook_system.hpp"
#include "ecsql/serialization.hpp"
#include "systems/draw_systems.hpp"
#include "systems/move_on_arrows.hpp"

#if defined(DEBUG) && !defined(NDEBUG)
void run_debug_functionality(ecsql::Ecsql& world) {
	ZoneScoped;
	DrawFPS(0, 0);

	for (int fkey = KEY_F1; fkey <= KEY_F10; fkey++) {
		bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT); 
		if (IsKeyPressed(fkey)) {
			const char *db_name = TextFormat("ecsql_world-backup%02d.sqlite3", fkey - KEY_F1 + 1);
			if (is_shift_down) {
				if (world.backup_into(db_name)) {
					std::cout << "Backed up into \"" << db_name << "\"" << std::endl;
				}
			}
			else {
				if (world.restore_from(db_name)) {
					std::cout << "Restored from \"" << db_name << "\"" << std::endl;
				}
			}
		}
	}
}
#endif

void game_loop(ecsql::Ecsql& world) {
	ZoneScoped;
	BeginDrawing();
	ClearBackground(RAYWHITE);

	world.update();

#if defined(DEBUG) && !defined(NDEBUG)
	run_debug_functionality(world);
#endif

	EndDrawing();
	FrameMark;
}

void game_loop(void *world) {
	game_loop(*(ecsql::Ecsql *) world);
}

int main(int argc, const char **argv) {
	configure_memory_hooks();

	const char *exe_dir_path = GetDirectoryPath(argv[0]);
	if (exe_dir_path && exe_dir_path[0]) {
		ChangeDirectory(exe_dir_path);
	}

	ecsql::assetio_initialize(argv[0], "com.gilzoide", "ecsql");

	const char *exe_file_name = GetFileName(argv[0]);
	TracySetProgramName(exe_file_name);
	InitWindow(800, 600, exe_file_name);

#if __EMSCRIPTEN__
	idbvfs_register(true);
#endif

	ecsql::Ecsql ecsql_world(getenv("ECSQL_DB"));
	
	// Components
	ecsql::RawComponent::foreach_static_linked_list([&](ecsql::RawComponent *component) {
		ecsql_world.register_component(*component);
	});
	ecsql::HookSystem::foreach_static_linked_list([&](ecsql::HookSystem *system) {
		ecsql_world.register_hook_system(*system);
	});

	// Systems
	register_move_on_arrows(ecsql_world);
	register_draw_systems(ecsql_world);

	// Scene
	const char *main_scene = argc >= 2 ? argv[1] : "assets/main.toml";
	bool loaded_main_scene = ecsql_world.inside_transaction([main_scene](ecsql::Ecsql& world) {
		load_scene_file(world, main_scene);
	});
	if (!loaded_main_scene) {
		std::cerr << "Could not load main scene '" << main_scene << "'. Bailing out." << std::endl;
		return 1;
	}

	SetTargetFPS(60);
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&game_loop, &ecsql_world, 0, 1);
#else
    while (!WindowShouldClose()) {
		game_loop(ecsql_world);
    }
#endif
	CloseWindow();

	ecsql::assetio_terminate();

	return 0;
}
