#pragma once

#include <sol/sol.hpp>

#include "../ecsql/world.hpp"

sol::state create_lua_state(ecsql::World& world);
