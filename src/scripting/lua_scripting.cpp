#include "lua_globals.h"
#include "lua_scripting.hpp"
#include "../memory.hpp"
#include "../ecsql/assetio.hpp"
#include "../ecsql/prepared_sql.hpp"
#include "../ecsql/system.hpp"

static void lua_register_system(sol::this_state L, ecsql::World& world, std::string_view name, sol::table table) {
	sol::function lua_function;
	std::vector<std::string> sqls;

	for (int i = 1; i <= table.size(); i++) {
		auto value = table[i];
		if (auto text = value.get<sol::optional<std::string_view>>()) {
			sqls.emplace_back(text.value());
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
			auto result = lua_function(sol::as_args(prepared_sql));
			if (!result.valid()) {
				throw result.get<sol::error>();
			}
		}
	});
}

static void lua_register_component(sol::this_state L, ecsql::World& world, std::string_view name, sol::table table) {
	std::vector<std::string> fields;
	for (int i = 1; i <= table.size(); i++) {
		auto value = table[i];
		if (auto text = value.get<sol::optional<std::string_view>>()) {
			fields.emplace_back(text.value());
		}
		else {
			sol::state_view state = L;
			luaL_error(L, "Unexpected object of type %s", lua_typename(L, (int) value.get_type()));
		}
	}
	std::string_view additional_schema = table.get<sol::optional<std::string_view>>("additional_schema").value_or(std::string_view());
	bool allow_duplicate = table.get<sol::optional<bool>>("allow_duplicate").value_or(false);
	world.register_component({
		name,
		fields,
		additional_schema,
		allow_duplicate,
	});
}

static ecsql::EntityID lua_create_entity(sol::this_state L, ecsql::World& world, sol::optional<std::string_view> name, sol::table table) {
	ecsql::EntityID entity_id = name ? world.create_entity(name.value()) : world.create_entity();
	for (auto [key, value] : table) {
		sol::table component_values = value;

		std::string sql = "INSERT INTO \"";
		sql += key.as<std::string_view>();
		sql += "\"(entity_id";
		int value_count = 0;
		for (auto [key, _] : component_values) {
			sql += ", ";
			sql += key.as<std::string_view>();
			value_count++;
		}
		sql += ") VALUES(?";
		for (int i = 0; i < value_count; i++) {
			sql += ", ?";
		}
		sql += ")";

		ecsql::PreparedSQL preparedSql(world.get_db().get(), sql);
		preparedSql.bind(1, entity_id);

		int i = 2;
		for (auto [_, value] : component_values) {
			switch (value.get_type()) {
				case sol::type::none:
				case sol::type::lua_nil:
					preparedSql.bind_null(i++);
					break;

				case sol::type::string:
					preparedSql.bind_text(i++, value.as<std::string_view>());
					break;

				case sol::type::number:
					preparedSql.bind_double(i++, value.as<lua_Number>());
					break;

				case sol::type::boolean:
					preparedSql.bind_bool(i++, value.as<bool>());
					break;

				default:
					luaL_error(L, "Unsupported type '%s' for component data", lua_typename(L, (int) value.get_type()));
			}
		}
		preparedSql();
	}
	return entity_id;
}

static ecsql::ExecutedSQL::RowIterator lua_prepared_sql_call(sol::this_state L, ecsql::PreparedSQL& prepared_sql, sol::variadic_args args) {
	int i = 1;
	for (auto value : args) {
		switch (value.get_type()) {
			case sol::type::none:
			case sol::type::lua_nil:
				prepared_sql.bind_null(i++);
				break;

			case sol::type::boolean:
				prepared_sql.bind_bool(i++, value);
				break;

			case sol::type::string:
				prepared_sql.bind_text(i++, value.get<std::string_view>());
				break;

			case sol::type::number:
				prepared_sql.bind_double(i++, value);
				break;

			default:
				luaL_error(L, "Unsupported type '%s' for SQL call", lua_typename(L, (int) value.get_type()));
		}
	}
	return ecsql::ExecutedSQL(prepared_sql.get_stmt()).begin();
}

static void register_usertypes(sol::state_view& state) {
	state.new_usertype<ecsql::World>(
		"World",
		sol::no_construction(),
		"register_system", lua_register_system,
		"register_component", lua_register_component,
		"create_entity", lua_create_entity
	);

	state.new_usertype<ecsql::PreparedSQL>(
		"PreparedSQL",
		sol::no_construction(),
		sol::meta_method::call, lua_prepared_sql_call
	);
}

static int string_replace(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    const char *p = luaL_checkstring(L, 2);
    const char *r = luaL_checkstring(L, 3);
    luaL_gsub(L, s, p, r);
    return 1;
}

LuaScripting::LuaScripting(ecsql::World& world)
	: state(lua_allocf ? sol::state(sol::default_at_panic, lua_allocf) : sol::state())
	, world(world)
{
	state.open_libraries();
	state["string"]["replace"] = string_replace;

	auto ecsql_namespace = state["ecsql"].get_or_create<sol::table>();
	ecsql_namespace["file_exists"] = [](const char *filename) -> bool {
		return PHYSFS_exists(filename);
	};
	ecsql_namespace["file_base_dir"] = PHYSFS_getBaseDir;
	ecsql_namespace["loadfile"] = [](sol::this_state L, const char *filename) -> std::pair<sol::object, sol::object> {
		auto load_result = ecsql::safe_load_lua_script(L, filename);
		if (load_result.valid()) {
			return  {
				sol::stack_object(L, -1),
				sol::lua_nil,
			};
		}
		else {
			return  {
				sol::lua_nil,
				sol::stack_object(L, -1),
			};
		}
	};

	register_usertypes(state);
	state["world"] = &world;

	auto result = state.do_string(std::string_view(lua_globals, lua_globals_size), "lua_globals.lua");
	if (!result.valid()) {
		throw result.get<sol::error>();
	}
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
