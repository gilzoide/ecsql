#include <iostream>

#include <sqlite3.h>
#include "benchmark.hpp"
#include "component.hpp"
#include "ecsql.hpp"
#include "SQLRow.hpp"
#include "raylib.h"

using namespace std;
using namespace ecsql;

void game_loop(Ecsql& world) {
	BeginDrawing();
	ClearBackground(RAYWHITE);

	{
		Benchmark _("World Update");
		world.update();
	}

	EndDrawing();
}

int main(int argc, const char **argv) {
	Ecsql world(getenv("ECSQL_DB"));

	// Components
	Component rectangle = Component::from_type<Rectangle>("rectangle");
	Component color = Component::from_type<Color>("color");
	world.register_component(rectangle);
	world.register_component(color);

	// Systems
	world.register_system("draw_rects", "SELECT x, y, width, height, r, g, b, a FROM rectangle NATURAL JOIN color", [](const SQLRow &row) {
		auto [rect, color] = row.get<Rectangle, Color>(0);
		DrawRectangleRec(rect, color);
	});

	const int ENTITIES = 10'000;
	// const int ENTITIES = 1'000;
	// const int ENTITIES = 10;
	cout << "Usando " << ENTITIES << " entidades" << endl;
	{
		Benchmark b("Insert entities");
		world.inside_transaction([&] {
			for (int i = 0; i < ENTITIES; i++) {
				world.create_entity();
			}
		});
	}
	{
		Benchmark b("Insert Rects+Color");
		world.inside_transaction([&](sqlite3 *db) {
			PreparedSQL insert_rectangle(db, rectangle.insert_sql());
			PreparedSQL insert_color(db, color.insert_sql());
			for (int i = 0; i < ENTITIES; i++) {
				insert_rectangle.reset()
					.bind_all(1, i, rand() % 800, rand() % 600, rand() % 5 + 5, rand() % 5 + 5);
				int res = insert_rectangle.step();
				if (res != SQLITE_DONE) {
					cout << sqlite3_errmsg(db) << endl;
					break;
				}
				
				insert_color.reset()
					.bind_all(1, i, rand() % 256, rand() % 256, rand() % 256, rand() % 256);
				res = insert_color.step();
				if (res != SQLITE_DONE) {
					cout << sqlite3_errmsg(db) << endl;
					break;
				}
			}
		});
	}
	
	InitWindow(800, 600, "ECSQL");
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		game_loop(world);
	}
	CloseWindow();

	return 0;
}
