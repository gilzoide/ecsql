#pragma once

#include <string>
#include <vector>

#include "PreparedSQL.hpp"
#include "ecsql.hpp"
#include "entity.hpp"
#include "reflect.hpp"

typedef struct sqlite3 sqlite3;

namespace ecsql {

class Component {
public:
	Component(const std::string& name, const std::vector<std::string>& fields);
	Component(const std::string& name, std::vector<std::string>&& fields);

	template<typename T>
	static Component from_type() {
		std::string component_name = std::string(reflect::type_name<T>());
		return from_type<T>(component_name);
	}
	
	template<typename T>
	static Component from_type(const std::string& component_name) {
		std::vector<std::string> fields;
		reflect::for_each<T>([&](auto I) {
			fields.push_back(std::string(reflect::member_name<I, T>()));
		});
		return Component(component_name, fields);
	}

	void prepare(sqlite3 *db);

	std::string schema_sql() const;
	std::string insert_sql() const;
	std::string update_sql() const;

	template<typename... Types>
	void insert(Entity entity, Types... values) {
		insert_stmt.reset().bind(1, entity, values...).step_single();
	}
	
	template<typename... Types>
	void update(Entity entity, Types... values) {
		update_stmt.reset().bind(1, entity, values...).step_single();
	}

protected:
	std::string name;
	std::vector<std::string> fields;

	PreparedSQL insert_stmt;
	PreparedSQL update_stmt;
};

}
