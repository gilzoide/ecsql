#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <reflect>

#include "entity.hpp"
#include "prepared_sql.hpp"

typedef struct sqlite3 sqlite3;

namespace ecsql {

class RawComponent {
public:
	RawComponent(std::string_view name, const std::vector<std::string>& fields);
	RawComponent(std::string_view name, std::vector<std::string>&& fields);

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

template<typename T>
class Component : public RawComponent {
	constexpr static std::vector<std::string> get_fields() {
		std::vector<std::string> fields;
		reflect::for_each<T>([&](auto I) {
			fields.push_back(std::string(reflect::member_name<I, T>()));
		});
		return fields;
	}
	
public:
	Component() : Component(reflect::type_name<T>()) {}
	Component(std::string_view name) : RawComponent(name, get_fields()) {}
};

}
