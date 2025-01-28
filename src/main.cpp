#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <idbvfs.h>
#endif
#include <raylib.h>
#include <sol/sol.hpp>
#include <tracy/Tracy.hpp>

#include "final_schema.h"
#include "memory.hpp"
#include "sqlite_functions.hpp"
#include "ecsql/assetio.hpp"
#include "ecsql/component.hpp"
#include "ecsql/hook_system.hpp"
#include "ecsql/screen.hpp"
#include "ecsql/serialization.hpp"
#include "ecsql/world.hpp"
#include "scripting/lua_scripting.hpp"
#include "systems/bake_position.hpp"
#include "systems/bake_random_screen_position.hpp"
#include "systems/destroy_on_out_of_screen.hpp"
#include "systems/draw_systems.hpp"
#include "systems/move_on_arrows.hpp"
#include "systems/screen_rect.hpp"
#include "systems/spawn_at_most.hpp"
#include "systems/spawn_scene_on_key.hpp"
#include "systems/yoga.hpp"

#if defined(DEBUG) && !defined(NDEBUG)
void run_debug_functionality(ecsql::World& world) {
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

void game_loop(ecsql::World& world) {
	ZoneScoped;
	BeginDrawing();
	ClearBackground(RAYWHITE);

	if (IsWindowResized()) {
		world.on_window_resized(GetScreenWidth(), GetScreenHeight());
	}

	float time_delta = GetFrameTime();
	world.update(time_delta);

#if defined(DEBUG) && !defined(NDEBUG)
	run_debug_functionality(world);
#endif

	EndDrawing();
	FrameMark;
}

void game_loop(void *world) {
	game_loop(*(ecsql::World *) world);
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
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 600, exe_file_name);

#if __EMSCRIPTEN__
	idbvfs_register(true);
#endif

	ecsql::World world(getenv("ECSQL_DB"));
	world.on_window_resized(GetScreenWidth(), GetScreenHeight());
	register_sqlite_functions(world);

	LuaScripting lua(world);

	// Components
	ecsql::Component::foreach_static_linked_list([&](ecsql::Component *component) {
		world.register_component(*component);
	});
	ecsql::HookSystem::foreach_static_linked_list([&](ecsql::HookSystem *system) {
		world.register_hook_system(*system);
	});
	world.execute_sql_script(final_schema);

	// Systems
	register_update_screen_rect(world);
	register_spawn_scene_on_key(world);
	register_spawn_at_most(world);
	register_bake_random_screen_position_system(world);
	register_bake_position_system(world);
	register_move_on_arrows(world);
	register_destroy_on_out_of_screen(world);
	register_update_yoga(world);
	register_draw_systems(world);

	// Scene
	const char *main_scene = argc >= 2 ? argv[1] : "main.lua";
	bool loaded_main_scene = world.inside_transaction([&](ecsql::World& world) {
		ecsql::do_lua_script(lua, main_scene);
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
		game_loop(world);
    }
#endif
	CloseWindow();

	ecsql::assetio_terminate();

	return 0;
}
