#pragma once

#include <sol/sol.hpp>

#include "../ecsql/world.hpp"

class LuaScripting {
public:
	LuaScripting(ecsql::World& world);
	~LuaScripting();

	operator lua_State *() const;
	operator sol::state_view() const;

private:
	ecsql::World& world;
	sol::state state;
};
