#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <raylib.h>
#include <tracy/Tracy.hpp>

#include "components/raylib_components.hpp"
#include "components/tags.hpp"
#include "ecsql/ecsql.hpp"
#include "ecsql/hook_system.hpp"
#include "flyweights/model_flyweight.hpp"
#include "flyweights/texture_flyweight.hpp"
#include "systems/draw_systems.hpp"
#include "systems/rotate_on_hover.hpp"

#if defined(DEBUG) && !defined(NDEBUG)
void run_debug_functionality(ecsql::Ecsql& world) {
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
	const char *exe_dir_path = GetDirectoryPath(argv[0]);
	if (exe_dir_path && exe_dir_path[0]) {
		ChangeDirectory(exe_dir_path);
	}

	const char *exe_file_name = GetFileName(argv[0]);
	TracySetProgramName(exe_file_name);
	InitWindow(800, 600, exe_file_name);

	ecsql::Ecsql ecsql_world(getenv("ECSQL_DB"));
	
	// Components
	ecsql::RawComponent::foreach_static_linked_list([&](auto component) {
		ecsql_world.register_component(*component);
	});
	ecsql::HookSystem::foreach_static_linked_list([&](auto system) {
		ecsql_world.register_hook_system(*system);
	});

	// Systems
	register_rotate_on_hover_systems(ecsql_world);
	register_draw_systems(ecsql_world);

	// Scene
	ecsql_world.inside_transaction([](ecsql::Ecsql& world) {
		for (int i = 0; i < 100; i++) {
			Entity img1 = world.create_entity();
			TextureFlyweight.component.insert(img1, "assets/textures/chick.png");
			PositionComponent.insert(img1, Vector2 { 0 + 4*(float) i, 0 + 4*(float) i });
		}
		
		for (int i = 0; i < 100; i++) {
			Entity img2 = world.create_entity();
			RotateOnHover.insert(img2);
			TextureFlyweight.component.insert(img2, "assets/textures/chick.png");
			RectangleComponent.insert(img2, Rectangle { 150 + 4*(float) i, 0 + 4*(float) i, 200, 200 });
			RotationComponent.insert(img2, 0, 0, 45);
			ColorComponent.insert(img2, LIME);
		}

		Entity cube = world.create_entity();
		ModelFlyweight.component.insert(cube, "assets/models/cube.obj");
		PositionComponent.insert(cube, 0, 0, 0);
		ColorComponent.insert(cube, SKYBLUE);
	});

	SetTargetFPS(60);
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(&game_loop, &ecsql_world, 0, 1);
#else
    while (!WindowShouldClose()) {
		game_loop(ecsql_world);
    }
#endif
	CloseWindow();

	return 0;
}
