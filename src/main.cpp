#include <cstdlib>
#include <iostream>
#include <cmath>

#include <sqlite3.h>
#include <string>
#include "components/texture_reference.hpp"
#include "ecsql/benchmark.hpp"
#include "ecsql/component.hpp"
#include "ecsql/system.hpp"
#include "ecsql/ecsql.hpp"
#include "flyweights/texture_flyweight.hpp"
#include "raylib.h"

using namespace ecsql;

struct ColoredRectangle {
	Rectangle rect;
	Color color;

	void Draw() const {
		DrawRectangleRec(rect, color);
	}
};

void game_loop(Ecsql& world, const std::vector<ColoredRectangle>& arrayzao) {
	BeginDrawing();
	ClearBackground(RAYWHITE);

	static int last_second = -1;
	static double ecs_ms;
	static double arrayzao_ms;
	static bool paused = false;
	int current_second = floor(GetTime());
	bool update_metrics = !paused && current_second > last_second;
	last_second = current_second;

	if (IsKeyPressed(KEY_SPACE)) {
		paused = !paused;
	}

	{
		Benchmark ecs_benchmark("Array Update", false);
		for (auto& it : arrayzao) {
			it.Draw();
		}
		if (update_metrics) {
			arrayzao_ms = ecs_benchmark.get_duration_ms();
		}
	}

	{
		Benchmark ecs_benchmark("World Update", false);
		world.update();
		if (update_metrics) {
			ecs_ms = ecs_benchmark.get_duration_ms();
		}
	}
	
	DrawRectangle(0, 0, 300, 35 * 3, Color { 0, 0, 0, static_cast<unsigned char>(0.8f * 255) });
	DrawText(TextFormat("ECS: %lf ms", ecs_ms), 5, 0, 30, WHITE);
	DrawText(TextFormat("Array: %lf ms", arrayzao_ms), 5, 35, 30, WHITE);
	DrawText(TextFormat("Ratio: %lf", ecs_ms / arrayzao_ms), 5, 75, 30, WHITE);

	EndDrawing();
}

int main(int argc, const char **argv) {
	const char *exe_dir_path = GetDirectoryPath(argv[0]);
	if (exe_dir_path && exe_dir_path[0]) {
		ChangeDirectory(exe_dir_path);
	}

	InitWindow(800, 600, "ECSQL");

	Ecsql ecsql_world(getenv("ECSQL_DB"));

	// Components
	Component rectangle = Component::from_type<Rectangle>("rectangle");
	Component color = Component::from_type<Color>("color");
	ecsql_world.register_component(rectangle);
	ecsql_world.register_component(color);
	
	TextureReference::register_component(ecsql_world);

	// Systems
	ecsql_world.register_system({
		"draw_rects",
		"SELECT x, y, width, height, r, g, b, a FROM rectangle INNER JOIN color USING(entity_id)",
		[](auto& sql) {
			for (SQLRow row : sql) {
				row.get<ColoredRectangle>(0).Draw();
			}
		}
	});

	ecsql_world.register_system({
		"draw_image",
		"SELECT path FROM TextureReference",
		[](auto& sql) {
			for (SQLRow row : sql) {
				auto texref = row.get<TextureReference>(0).get();
				DrawTexture(texref, 200, 200, WHITE);
			}
		}
	});

	ecsql_world.inside_transaction([&] {
		auto img_id = ecsql_world.create_entity("texture_test");
		TextureReferenceComponent.insert(img_id, "textures/chick.png");
	});

	const int ENTITIES = 10'000;
	// const int ENTITIES = 1'000;
	// const int ENTITIES = 100;
	std::vector<ColoredRectangle> entity_vector;
	std::cout << "Usando " << ENTITIES << " entidades" << std::endl;
	{
		Benchmark b("Insert entities");
		ecsql_world.inside_transaction([&] {
			for (int i = 0; i < ENTITIES; i++) {
				ecsql_world.create_entity();
			}
		});
	}
	{
		Benchmark b("Create Rects+Color in array");
		for (int i = 0; i < ENTITIES; i++) {
			entity_vector.emplace_back(ColoredRectangle {
				.rect = {
					static_cast<float>(rand() % 800),
					static_cast<float>(rand() % 600),
					static_cast<float>(rand() % 5 + 5),
					static_cast<float>(rand() % 5 + 5),
				},
				.color = {
					static_cast<unsigned char>(rand() % 256),
					static_cast<unsigned char>(rand() % 256),
					static_cast<unsigned char>(rand() % 256),
					255,
				},
			});
		}
	}
	{
		Benchmark b("Insert Rects+Color");
		ecsql_world.inside_transaction([&] {
			for (int entity_id = 0; entity_id < ENTITIES; entity_id++) {
				rectangle.insert(entity_id, entity_vector[entity_id].rect);
				color.insert(entity_id, entity_vector[entity_id].color);
			}
		});
	}
	
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		game_loop(ecsql_world, entity_vector);
	}
	CloseWindow();

	return 0;
}
