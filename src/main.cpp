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
		"x FLOAT",
		"y FLOAT",
		"z FLOAT",
	});
	world.register_component(position);
	world.register_component<TesteForeach>();
	auto pos_system = world.register_system("SELECT entity_id, x, y, z FROM position", [](const SQLRow &row) {
		// process_row(row.column_int64(0), row.column_double(1), row.column_double(2), row.column_double(3));
		// TesteForeach t = row.get<TesteForeach>(0);
		auto [id, x, y] = row.get<int, double, double>(0);
		// cout << y << endl;
		// t.printa();
	});

	const int ENTITIES = 10'000;
	// const int ENTITIES = 1'000;
	// const int ENTITIES = 10;
	cout << "Usando " << ENTITIES << " entidades" << endl;
	{
		Benchmark b("Insert entities");
		world.inside_transaction([&world] {
			for (int i = 0; i < ENTITIES; i++) {
				world.create_entity();
			}
		});
	}
	{
		Benchmark b("Insert positions");
		world.inside_transaction([](sqlite3 *db) {
			PreparedSQL insert_position(db, "INSERT INTO position(entity_id, x, y, z) VALUES(?, ?, ?, ?)");
			for (int i = 0; i < ENTITIES; i++) {
				insert_position.reset()
					.bind(1, i)
					.bind(2, 0)
					.bind(3, rand() % 3)
					.bind(4, 10);
				int res = insert_position.step();
				if (res != SQLITE_DONE) {
					cout << sqlite3_errmsg(db) << endl;
					break;
				}
			}
		});
	}
	
	{
		Benchmark b("Query positions");
		pos_system();
	}
	// {
	//     Benchmark b("Query positions (transaction)");
	//     world.inside_transaction(pos_system);
	// }

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
