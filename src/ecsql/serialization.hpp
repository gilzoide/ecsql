#pragma once

#include <istream>
#include <string>
#include <string_view>

#include <toml++/toml.hpp>

#include "ecsql.hpp"


namespace ecsql {

std::string load_scene(const toml::table& toml);
std::string load_scene(std::string_view source, std::string_view source_path = {});
std::string load_scene(std::istream& stream, std::string_view source_path = {});
std::string load_scene_file(std::string_view file_name);

std::string load_components(const toml::table& toml);
std::string load_components(std::string_view source, std::string_view source_path = {});
std::string load_components(std::istream& stream, std::string_view source_path = {});
std::string load_components_file(std::string_view file_name);

}
