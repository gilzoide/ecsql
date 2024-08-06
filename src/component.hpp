#pragma once

#include <string>
#include <vector>
#include "reflect.hpp"

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

	std::string schema_sql() const;
	std::string insert_sql() const;

	std::string name;
	std::vector<std::string> fields;
};

}
