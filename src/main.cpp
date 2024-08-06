#include <iostream>

#include <sqlite3.h>
#include "benchmark.hpp"
#include "component.hpp"
#include "ecsql.hpp"
#include "SQLRow.hpp"

using namespace std;
using namespace ecsql;

struct TesteForeach {
	sqlite3_int64 outro_id;
	double x;
	double y;
	double z;

	void Metodo() {}
	void Metodo2() {}
	void printa() const {
		cout << "(" << x << ", " << y << ", " << z << ")" << endl;
	}
};

double process_row(entity_id id, double x, double y, double z) {
	return x + y + z;
}

int main(int argc, const char **argv) {
	Ecsql world(getenv("ECSQL_DB"));
	Component position("position", {
		"x",
		"y",
		"z",
	});
	world.register_component(position);
	world.register_component<TesteForeach>();
	world.register_system("teste1", "SELECT entity_id, x, y, z FROM position", [](const SQLRow &row) {
		// process_row(row.column_int64(0), row.column_double(1), row.column_double(2), row.column_double(3));
		// TesteForeach t = row.get<TesteForeach>(0);
		auto [id, x, y, z] = row.get<int, double, double, double>(0);
		process_row(id, x, y, z);
		// cout << y << endl;
		// t.printa();
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
		Benchmark b("Insert positions");
		world.inside_transaction([&](sqlite3 *db) {
			PreparedSQL insert_position(db, position.insert_sql());
			for (int i = 0; i < ENTITIES; i++) {
				insert_position.reset()
					.bind_all(1, i, 0, rand() % 3, 10);
				int res = insert_position.step();
				if (res != SQLITE_DONE) {
					cout << sqlite3_errmsg(db) << endl;
					break;
				}
			}
		});
	}
	
	{
		for (int i = 0; i < 10; i++) {
			Benchmark b("Query positions");
			world.update();
		}
	}

	{
		TesteForeach lista[ENTITIES];
		for (int i = 0; i < ENTITIES; i++) {
			lista[i] = {
				.outro_id = i,
				.x = 0,
				.y = 0,
				.z = 0,
			};
		}

		Benchmark b("Foreach");
		for (int i = 0; i < ENTITIES; i++) {
			process_row(lista[i].outro_id, lista[i].x, lista[i].y, lista[i].z);
		}
	}
	return 0;
}
