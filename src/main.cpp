#include <cstdlib>
#include <string>

#include <raylib.h>
#include <tracy/Tracy.hpp>

#include "components/raylib_components.hpp"
#include "components/texture_reference.hpp"
#include "ecsql/ecsql.hpp"
#include "flyweights/texture_flyweight.hpp"
#include "systems/draw_systems.hpp"

void game_loop(ecsql::Ecsql& world) {
	BeginDrawing();
	ClearBackground(RAYWHITE);

	world.update();

#if DEBUG
	DrawFPS(0, 0);
#endif

	EndDrawing();
	FrameMark;
}

int main(int argc, const char **argv) {
	TracySetProgramName(argv[0]);

	const char *exe_dir_path = GetDirectoryPath(argv[0]);
	if (exe_dir_path && exe_dir_path[0]) {
		ChangeDirectory(exe_dir_path);
	}

	InitWindow(800, 600, "ECSQL");

	ecsql::Ecsql ecsql_world(getenv("ECSQL_DB"));
	
	// Components
	register_raylib_components(ecsql_world);
	TextureReference::register_component(ecsql_world);

	// Systems
	register_draw_systems(ecsql_world);

	// Scene
	ecsql_world.inside_transaction([](auto& world) {
		Entity img1 = world.create_entity();
		TextureReferenceComponent.insert(img1, "textures/chick.png");
		RectangleComponent.insert(img1, Rectangle { 100, 100, 200, 200 });
		
		Entity img2 = world.create_entity();
		TextureReferenceComponent.insert(img2, "textures/chick.png");
		RectangleComponent.insert(img2, Rectangle { 400, 100, 200, 200 });
		ColorComponent.insert(img2, LIME);
	});

	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		game_loop(ecsql_world);
	}
	CloseWindow();

	return 0;
}
