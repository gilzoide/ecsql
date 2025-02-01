#include <raymath.h>

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

static ecsql::EntityID lua_create_entity(sol::this_state L, ecsql::World& world, std::optional<std::string_view> name, sol::table table) {
	auto parent_id = table.get<std::optional<ecsql::EntityID>>("parent_id");
	ecsql::EntityID entity_id = world.create_entity(name, parent_id);
	for (auto [key, value] : table) {
		if (key.as<std::string_view>() == "parent_id") {
			continue;
		}

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

static ecsql::ExecutedSQL lua_prepared_sql_call(sol::this_state L, ecsql::PreparedSQL& prepared_sql, sol::variadic_args args) {
	prepared_sql.reset();
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
	return prepared_sql.execute();
}

static sol::object lua_sql_row_get(sol::this_state L, ecsql::ExecutedSQL::RowIterator& it, int index) {
	ecsql::SQLRow row = *it;
	index--;
	if (index < 0 || index >= row.column_count()) {
		return sol::lua_nil;
	}

	switch (row.column_type(index)) {
		case SQLITE_INTEGER:
			return sol::make_object(L, row.column_int64(index));

		case SQLITE_FLOAT:
			return sol::make_object(L, row.column_double(index));

		case SQLITE_TEXT:
			return sol::make_object(L, row.column_text(index));

		case SQLITE_BLOB: {
			auto blob = row.column_blob(index);
			return sol::make_object(L, std::string_view((const char *) blob.data(), blob.size()));
		}

		default:
			return sol::lua_nil;
	}
}

static void register_usertypes(sol::state_view& state) {
	state.new_usertype<ecsql::World>(
		"World",
		sol::no_construction(),
		"register_system", lua_register_system,
		"register_component", lua_register_component,
		"create_entity", lua_create_entity,
		"prepare_sql", [](ecsql::World& world, std::string_view sql, sol::optional<bool> is_persistent) {
			return world.prepare_sql(sql, is_persistent.value_or(false));
		},
		"execute_sql", [](sol::this_state L, ecsql::World& world, std::string_view sql, sol::variadic_args args) {
			ecsql::PreparedSQL prepared_sql = world.prepare_sql(sql);
			return lua_prepared_sql_call(L, prepared_sql, args);
		},
		"execute_sql_script", &ecsql::World::execute_sql_script
	);

	state.new_usertype<ecsql::PreparedSQL>(
		"PreparedSQL",
		sol::no_construction(),
		sol::meta_method::call, lua_prepared_sql_call
	);

	state.new_usertype<ecsql::ExecutedSQL>(
		"ExecutedSQL",
		sol::no_construction(),
		sol::meta_method::call, [](sol::this_state L, ecsql::ExecutedSQL& executed_sql, sol::variadic_args args) -> sol::object {
			ecsql::ExecutedSQL::RowIterator it;
			if (args.size() >= 2 && args[1].is<ecsql::ExecutedSQL::RowIterator>()) {
				it = args[1].as<ecsql::ExecutedSQL::RowIterator>();
				++it;
			}
			else {
				it = executed_sql.begin();
			}

			if (it) {
				return sol::make_object(L, it);
			}
			else {
				return sol::lua_nil;
			}
		}
	);

	state.new_usertype<ecsql::ExecutedSQL::RowIterator>(
		"SQLRowIterator",
		sol::meta_method::index, lua_sql_row_get,
		sol::meta_method::length, [](ecsql::ExecutedSQL::RowIterator& it) { return (*it).column_count(); },
		"unpack", state["table"]["unpack"].get<sol::object>()
	);

	state.new_usertype<Vector2>(
		"Vector2",
		sol::call_constructor, sol::factories(
			[]() -> Vector2 { return {}; },
			[](float xy) -> Vector2 { return { .x = xy, .y = xy }; },
			[](float x, float y) -> Vector2 { return { .x = x, .y = y }; }
		),
		"x", &Vector2::x,
		"y", &Vector2::y,
		"normalized", Vector2Normalize,
		"unpack", [](Vector2 v) { return std::make_pair(v.x, v.y); },
		sol::meta_method::to_string, [](Vector2 v) { return std::format("({}, {})", v.x, v.y); }
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
