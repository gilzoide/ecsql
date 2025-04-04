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

#include "memory.hpp"
#include "sqlite_functions.hpp"
#include "ecsql/additional_sql.hpp"
#include "ecsql/assetio.hpp"
#include "ecsql/component.hpp"
#include "ecsql/hook_system.hpp"
#include "ecsql/world.hpp"
#include "physics/physics.hpp"
#include "scripting/lua_scripting.hpp"
#include "systems/draw_systems.hpp"
#include "systems/key_handler.hpp"
#include "systems/yoga.hpp"

#if defined(DEBUG) && !defined(NDEBUG)
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
}
#endif

static void log_function(void *, int error, const char *message) {
	if (error != SQLITE_SCHEMA) {
		std::cerr << "[sqlite3] (" << error << ") " << message << std::endl;
	}
}

void game_loop(ecsql::World& world) {
	ZoneScoped;
	{
		ZoneScopedN("BeginDrawing");
		BeginDrawing();
	}

	if (IsWindowResized()) {
		world.on_window_resized(GetScreenWidth(), GetScreenHeight());
	}

	float time_delta = GetFrameTime();
	world.update(time_delta);

#if defined(DEBUG) && !defined(NDEBUG)
	run_debug_functionality(world);
#endif

	{
		ZoneScopedN("EndDrawing");
		EndDrawing();
	}
	FrameMark;
}

void game_loop(void *world) {
	game_loop(*(ecsql::World *) world);
}

int main(int argc, const char **argv) {
	configure_memory_hooks();
	sqlite3_config(SQLITE_CONFIG_LOG, log_function, nullptr);

	const char *exe_dir_path = GetDirectoryPath(argv[0]);
	if (exe_dir_path && exe_dir_path[0]) {
		ChangeDirectory(exe_dir_path);
	}

	ecsql::assetio_initialize(argv[0], "com.gilzoide", "ecsql");

	const char *exe_file_name = GetFileName(argv[0]);
	TracySetProgramName(exe_file_name);
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 600, exe_file_name);

#if __EMSCRIPTEN__
	idbvfs_register(true);
#endif

	ecsql::World world(getenv("ECSQL_DB"));
	world.on_window_resized(GetScreenWidth(), GetScreenHeight());
	register_sqlite_functions(world);

	// Components
	ecsql::Component::foreach_static_linked_list([&](ecsql::Component *component) {
		world.register_component(*component);
	});
	ecsql::HookSystem::foreach_static_linked_list([&](ecsql::HookSystem *system) {
		world.register_hook_system(*system);
	});
	ecsql::AdditionalSQL::foreach_static_linked_list([&](ecsql::AdditionalSQL *additional_sql) {
		world.execute_sql_script(additional_sql->get_sql().c_str());
	});

	// Systems
	register_key_handler(world);
	register_update_yoga(world);
	register_draw_systems(world);

	// Other engine features, must be created after world components are registered
	LuaScripting lua(world);
	Physics physics(world);

	// Scene
	const char *main_scene = argc >= 2 ? argv[1] : "main.lua";
	bool loaded_main_scene = world.inside_transaction([&](ecsql::World& world) {
		ecsql::do_lua_script(lua, main_scene);
	});
	if (!loaded_main_scene) {
		std::cerr << "Could not load main scene '" << main_scene << "'. Bailing out." << std::endl;
		return 1;
	}

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(&game_loop, &world, 0, 1);
#else
	while (!WindowShouldClose()) {
		game_loop(world);
	}
#endif
	CloseWindow();

	ecsql::assetio_terminate();

	return 0;
}
