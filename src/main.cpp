#include <cstdlib>
#include <iostream>
#include <string>

#include <raylib.h>
#include <sqlite3.h>
#include <tracy/Tracy.hpp>

#include "components/texture_reference.hpp"
#include "ecsql/benchmark.hpp"
#include "ecsql/component.hpp"
#include "ecsql/prepared_sql.hpp"
#include "ecsql/system.hpp"
#include "ecsql/ecsql.hpp"
#include "flyweights/texture_flyweight.hpp"

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
		[](auto& sql) {
			for (SQLRow row : sql) {
				row.get<ColoredRectangle>(0).Draw();
			}
		},
		"SELECT x, y, width, height, r, g, b, a FROM rectangle INNER JOIN color USING(entity_id)",
	});

	ecsql_world.register_system(System {
		"draw_image",
		[](auto& sql) {
			for (SQLRow row : sql) {
				auto texref = row.get<TextureReference>(0).get();
				DrawTexture(texref, 200, 200, WHITE);
			}
		},
		"SELECT path FROM TextureReference",
	});

	ecsql_world.register_system({
		"exchange_image",
		[](auto& select, auto& update) {
			if (IsKeyPressed(KEY_X)) {
				for (SQLRow row : select) {
					auto [id, texref] = row.get<Entity, TextureReference>(0);
					const char *new_texture = texref.path == "textures/chick.png"
						? "textures/chicken.png"
						: "textures/chick.png";
					update.reset().bind(1, new_texture, id).step_single();
				}
			}
		},
		"SELECT id, path FROM TextureReference",
		"UPDATE TextureReference SET path = ? WHERE id = ?",
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
