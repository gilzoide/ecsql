#include "lua_globals.h"
#include "lua_scripting.hpp"
#include "../memory.hpp"
#include "../ecsql/prepared_sql.hpp"
#include "../ecsql/system.hpp"

static void lua_register_system(sol::this_state L, ecsql::World& world, std::string_view name, sol::table table) {
	sol::function lua_function;
	std::vector<std::string> sqls;

	for (int i = 1; i <= table.size(); i++) {
		auto value = table[i];
		if (auto text = value.get<sol::optional<std::string_view>>()) {
			sqls.push_back(std::string(text.value()));
		}
		else if (lua_function) {
			luaL_error(L, "Unexpected object at index %d. Only strings and a single function are allowed.", i);
		}
		else if (auto f = value.get<sol::optional<sol::function>>()) {
			lua_function = f.value();
		}
	}

	if (!lua_function) {
		luaL_error(L, "Expected function in system's table definition.");
	}

	std::string prefixed_name = "lua.";
	prefixed_name += name,
	world.register_system({
		prefixed_name,
		sqls,
		[lua_function](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sql) {
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

static void lua_register_component(sol::this_state L, ecsql::World& world, std::string_view name, sol::table table, sol::optional<std::string_view> additional_schema) {
	std::vector<std::string> fields;
	for (int i = 1; i <= table.size(); i++) {
		auto value = table[i];
		if (auto text = value.get<sol::optional<std::string_view>>()) {
			fields.push_back(std::string(text.value()));
		}
		else {
			sol::state_view state = L;
			luaL_error(L, "Unexpected object of type %s", lua_typename(L, (int) value.get_type()));
		}
	}
	world.register_component({
		name,
		fields,
		additional_schema.value_or(std::string_view()),
	});
}

static void register_usertypes(sol::state_view& lua) {
	lua.new_usertype<ecsql::World>(
		"World",
		sol::no_construction(),
		"register_system", lua_register_system,
		"register_component", lua_register_component
	);

	lua.new_usertype<ecsql::PreparedSQL>(
		"PreparedSQL",
		sol::no_construction()
	);
}

LuaScripting::LuaScripting(ecsql::World& world)
	: state(lua_allocf ? sol::state(sol::default_at_panic, lua_allocf) : sol::state())
	, world(world)
{
	state.open_libraries();

	register_usertypes(state);
	state["world"] = &world;

	state.do_string(std::string_view(lua_globals, lua_globals_size), "LuaTeste");
}

LuaScripting::~LuaScripting() {
	world.remove_systems_with_prefix("lua.");
}

LuaScripting::operator lua_State *() const {
	return state;
}

LuaScripting::operator sol::state_view() const {
	return state;
}
