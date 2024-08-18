#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <reflect>

#include "entity.hpp"
#include "prepared_sql.hpp"

typedef struct sqlite3 sqlite3;

namespace ecsql {

class RawComponent {
public:
	RawComponent(std::string_view name, const std::vector<std::string>& fields, bool allow_duplicate = false);
	RawComponent(std::string_view name, std::vector<std::string>&& fields, bool allow_duplicate = false);

	void prepare(sqlite3 *db);

	// If `allow_duplicate`, entity_id is in index 1. Otherwise it's in index 0.
	int entity_id_index() const;
	// If `allow_duplicate`, first field index is 2. Otherwise it's in index 1.
	int first_field_index() const;

	std::string schema_sql() const;
	std::string insert_sql() const;
	std::string update_sql() const;

	template<typename... Types>
	void insert(Entity entity, Types&&... values) {
		insert_stmt.reset().bind(1, entity, std::forward<Types>(values)...).step_single();
	}
	
	template<typename... Types>
	void update(Entity entity, Types&&... values) {
		update_stmt.reset().bind(1, std::forward<Types>(values)..., entity).step_single();
	}

protected:
	std::string name;
	std::vector<std::string> fields;
	bool allow_duplicate;

	PreparedSQL insert_stmt;
	PreparedSQL update_stmt;
};

template<typename T>
class Component : public RawComponent {
	constexpr static std::vector<std::string> get_fields() {
		std::vector<std::string> fields;
		if constexpr (std::is_class_v<T>) {
			reflect::for_each<T>([&](auto I) {
				fields.push_back(std::string(reflect::member_name<I, T>()));
			});
		}
		else {
			fields.push_back("value");
		}
		return fields;
	}
	
public:
	Component() : Component(reflect::type_name<T>()) {}
	Component(std::string_view name) : RawComponent(name, get_fields()) {}
};

}
