#pragma once

#include <istream>
#include <string_view>

#include "ecsql.hpp"

namespace ecsql {

void load_scene(Ecsql& world, std::string_view source);
void load_scene(Ecsql& world, std::istream& stream, std::string_view source_path = {});

}
