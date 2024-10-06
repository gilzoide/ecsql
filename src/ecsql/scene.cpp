#include <toml++/toml.hpp>

#include "scene.hpp"

namespace ecsql {

static void load_scene(Ecsql& world, toml::parse_result toml) {
	for (auto&& [entity_name, entity_node] : toml) {
		Entity entity = world.create_entity(entity_name);
		if (toml::table *entity_table = entity_node.as_table()) {
			for (auto&& [component_name, component_node] : *entity_table) {
				if (toml::table *component_table = component_node.as_table()) {
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

					PreparedSQL prepared_sql(world.get_db(), sql, false);
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
				else {
					std::string error = "[Ecsql::load_scene] Expected component for entity named '";
					error += entity_name;
					error += '.';
					error += component_name;
					error += '\'';
					throw std::runtime_error(error);
				}
			}
		}
		else {
			std::string error = "[Ecsql::load_scene] Expected table for entity named '";
			error += entity_name;
			error += '\'';
			throw std::runtime_error(error);
		}
	}
}

void load_scene(Ecsql& world, std::string_view source) {
	load_scene(world, toml::parse(source));
}

void load_scene(Ecsql& world, std::istream& stream, std::string_view source_path) {
	load_scene(world, toml::parse(stream, source_path));
}

}
