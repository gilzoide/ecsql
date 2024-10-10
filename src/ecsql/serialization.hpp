#pragma once

#include <istream>
#include <string_view>

#include <toml++/toml.hpp>

#include "ecsql.hpp"


namespace ecsql {

void load_scene(Ecsql& world, const toml::table& toml);
void load_scene(Ecsql& world, std::string_view source, std::string_view source_path = {});
void load_scene(Ecsql& world, std::istream& stream, std::string_view source_path = {});
void load_scene_file(Ecsql& world, std::string_view file_name);

void load_components(Ecsql& world, const toml::table& toml);
void load_components(Ecsql& world, std::string_view source, std::string_view source_path = {});
void load_components(Ecsql& world, std::istream& stream, std::string_view source_path = {});
void load_components_file(Ecsql& world, std::string_view file_name);

}
