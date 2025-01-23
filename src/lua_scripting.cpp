#include "lua_globals.h"
#include "lua_scripting.hpp"
#include "memory.hpp"
#include "ecsql/prepared_sql.hpp"
#include "ecsql/system.hpp"

static void lua_register_system(sol::this_state L, ecsql::World& world, std::string_view name, sol::table table) {
	sol::function lua_function;
	std::vector<std::string> sqls;

	for (int i = 1; i <= table.size(); i++) {
		auto value = table[i];
		if (auto text = value.get<sol::optional<std::string_view>>()) {
			sqls.push_back(std::string(text.value()));
		}
		else if (auto f = value.get<sol::optional<sol::function>>()) {
			lua_function = f.value();
		}
	}

	if (!lua_function) {
		luaL_error(L, "Expected function in system's table definition");
	}

	world.register_system({
		name,
		sqls,
		[=](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sql) {
			lua_function.push();
			sol::state_view state = lua_function.lua_state();
			for (auto& it : prepared_sql) {
				sol::stack::push(state, it);
			}
			if (lua_pcall(state, prepared_sql.size(), 0, 0) != LUA_OK) {
				sol::stack_object error(state);
				std::cerr << "Error: " << error.as<const char *>() << std::endl;
				lua_pop(state, 1);
			}
		}
	});
}

static void register_usertypes(sol::state_view& lua) {
	lua.new_usertype<ecsql::World>(
		"World",
		sol::no_construction(),
		"register_system", lua_register_system
	);

	lua.new_usertype<ecsql::PreparedSQL>(
		"PreparedSQL",
		sol::no_construction()
	);
}

sol::state create_lua_state(ecsql::World& world) {
	sol::state lua = lua_allocf ? sol::state(sol::default_at_panic, lua_allocf) : sol::state();
	lua.open_libraries();

	register_usertypes(lua);
	lua["world"] = &world;

	lua.do_string(std::string_view(lua_globals, lua_globals_size), "LuaTeste");

	return lua;
}
