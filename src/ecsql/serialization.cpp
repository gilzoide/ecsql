#include <fstream>

#include "assetio.hpp"
#include "component.hpp"
#include "serialization.hpp"

namespace ecsql {

void load_scene(Ecsql& world, const toml::table& toml) {
	for (auto&& [entity_name, entity_node] : toml) {
		const toml::table *entity_table = entity_node.as_table();
		if (!entity_table) {
			std::string error = "[Ecsql::load_scene] Expected table for entity named '";
			error += entity_name;
			error += '\'';
			throw std::runtime_error(error);
		}

		Entity entity = world.create_entity(entity_name);
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

			std::string sql = "INSERT INTO ";
			sql += component_name;
			sql += "(entity_id";
			for (auto&& [column_name, _] : *component_table) {
				sql += ", ";
				sql += column_name;
			}
			sql += ") VALUES (?";
			for (int i = 0; i < component_table->size(); i++) {
				sql += ", ?";
			}
			sql += ")";

			PreparedSQL prepared_sql(world.get_db().get(), sql, false);
			int i = 1;
			prepared_sql.bind(i++, entity);
			for (auto&& [_, value_node] : *component_table) {
				if (auto bool_value = value_node.as_boolean()) {
					prepared_sql.bind(i++, bool_value->get());
				}
				else if (auto int_value = value_node.as_integer()) {
					prepared_sql.bind(i++, int_value->get());
				}
				else if (auto float_value = value_node.as_floating_point()) {
					prepared_sql.bind(i++, float_value->get());
				}
				else if (auto string_value = value_node.as_string()) {
					prepared_sql.bind(i++, (std::string_view) string_value->get());
				}
			}
			prepared_sql();
		}
	}
}

void load_scene(Ecsql& world, std::string_view source, std::string_view source_path) {
	load_scene(world, toml::parse(source, source_path));
}

void load_scene(Ecsql& world, std::istream& stream, std::string_view source_path) {
	load_scene(world, toml::parse(stream, source_path));
}

void load_scene_file(Ecsql& world, std::string_view file_name) {
	auto file_data = read_asset_data<std::string>(file_name.data());
	load_scene(world, file_data, file_name);
}

void load_components(Ecsql& world, const toml::table& toml) {
	for (auto [component_name, component_node] : toml) {
		const toml::table *component_table = component_node.as_table();
		if (!component_table) {
			std::string error = "[Ecsql::load_scene] Expected table for component named '";
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
			fields.emplace_back(field);
		}

		world.register_component({ component_name, fields });
	}
}

void load_components(Ecsql& world, std::string_view source, std::string_view source_path) {
	load_components(world, toml::parse(source, source_path));
}

void load_components(Ecsql& world, std::istream& stream, std::string_view source_path) {
	load_components(world, toml::parse(stream, source_path));
}

void load_components_file(Ecsql& world, std::string_view file_name) {
	auto file_data = read_asset_data<std::string>(file_name.data());
	load_components(world, file_data, file_name);
}

}
