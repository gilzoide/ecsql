#include <fstream>

#include "assetio.hpp"
#include "component.hpp"
#include "serialization.hpp"

namespace ecsql {

std::string load_scene(const toml::table& toml) {
	std::string sql;
	for (auto&& [entity_name, entity_node] : toml) {
		const toml::table *entity_table = entity_node.as_table();
		if (!entity_table) {
			std::string error = "[Ecsql::load_scene] Expected table for entity named '";
			error += entity_name;
			error += '\'';
			throw std::runtime_error(error);
		}

		// Insert entity
		sql += "INSERT INTO entity(name) VALUES(";
		if (entity_name.empty()) {
			sql += "NULL";
		}
		else {
			sql += '\'';
			sql += entity_name.str();
			sql += '\'';
		}
		sql += ");\n";

		for (auto&& [component_name, component_node] : *entity_table) {
			const toml::table *component_table = component_node.as_table();
			if (!component_table) {
				std::string error = "[Ecsql::load_scene] Expected component for entity named '";
				error += entity_name;
				error += '.';
				error += component_name;
				error += '\'';
				throw std::runtime_error(error);
			}

			sql += "INSERT INTO ";
			sql += component_name;
			sql += "(entity_id";
			for (auto&& [column_name, _] : *component_table) {
				sql += ", ";
				sql += column_name;
			}
			sql += ") VALUES (last_insert_rowid()";
			for (auto&& [column_name, value_node] : *component_table) {
				sql += ", ";
				if (auto bool_value = value_node.as_boolean()) {
					sql += bool_value->get() ? "TRUE" : "FALSE";
				}
				else if (auto int_value = value_node.as_integer()) {
					sql += std::to_string(int_value->get());
				}
				else if (auto float_value = value_node.as_floating_point()) {
					sql += std::to_string(float_value->get());
				}
				else if (auto string_value = value_node.as_string()) {
					sql += '\'';
					sql += string_value->get();
					sql += '\'';
				}
				else {
					std::string error = "[Ecsql::load_scene] Expected bool, int, float or string for column named '";
					error += entity_name;
					error += '.';
					error += component_name;
					error += '.';
					error += column_name;
					error += '\'';
					throw std::runtime_error(error);
				}
			}
			sql += ");\n";
		}
	}
	return sql;
}

std::string load_scene(std::string_view source, std::string_view source_path) {
	return load_scene(toml::parse(source, source_path));
}

std::string load_scene(std::istream& stream, std::string_view source_path) {
	return load_scene(toml::parse(stream, source_path));
}

std::string load_scene_file(std::string_view file_name) {
	auto file_data = read_asset_data<std::string>(file_name.data());
	return load_scene(file_data, file_name);
}

std::string load_components(const toml::table& toml) {
	std::string sql;
	for (auto [component_name, component_node] : toml) {
		const toml::table *component_table = component_node.as_table();
		if (!component_table) {
			std::string error = "[Ecsql::load_components] Expected table for component named '";
			error += component_name;
			error += '\'';
			throw std::runtime_error(error);
		}

		std::vector<std::string> fields;
		for (auto [field_name, value_node] : *component_table) {
			std::string field { field_name };
			if (auto bool_value = value_node.as_boolean()) {
				field += " DEFAULT ";
				field += bool_value ? "1" : "0";
			}
			else if (auto int_value = value_node.as_integer()) {
				field += " DEFAULT ";
				field += std::to_string(int_value->get());
			}
			else if (auto float_value = value_node.as_floating_point()) {
				field += " DEFAULT ";
				field += std::to_string(float_value->get());
			}
			else if (auto string_value = value_node.as_string()) {
				field += string_value->get();
			}
			else {
				std::string error = "[Ecsql::load_components] Expected bool, int, float or string for field named '";
				error += component_name;
				error += '.';
				error += field_name;
				error += '\'';
				throw std::runtime_error(error);
			}
			fields.emplace_back(field);
		}

		sql += RawComponent(component_name, fields).schema_sql();
		sql += '\n';
	}
	return sql;
}

std::string load_components(std::string_view source, std::string_view source_path) {
	return load_components(toml::parse(source, source_path));
}

std::string load_components(std::istream& stream, std::string_view source_path) {
	return load_components(toml::parse(stream, source_path));
}

std::string load_components_file(std::string_view file_name) {
	auto file_data = read_asset_data<std::string>(file_name.data());
	return load_components(file_data, file_name);
}

}
