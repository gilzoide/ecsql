#pragma once

#include <type_traits>
#include <utility>

#include <sqlite3.h>

#include "sql_value.hpp"

namespace ecsql {

void set_function_result(sqlite3_context *ctx, bool value);
void set_function_result(sqlite3_context *ctx, sqlite3_int64 value);
void set_function_result(sqlite3_context *ctx, double value);
void set_function_result(sqlite3_context *ctx, const char *value);
void set_function_result(sqlite3_context *ctx, std::string_view value);
void set_function_result(sqlite3_context *ctx, const std::string& value);

namespace detail {
	template<typename RetType, typename... Args, size_t... I>
	void sql_function_entrypoint(sqlite3_context *ctx, int argc, sqlite3_value **argv, std::index_sequence<I...>) {
		RetType (*fn)(Args...) = reinterpret_cast<RetType (*)(Args...)>(sqlite3_user_data(ctx));
		if constexpr (std::is_void_v<RetType>) {
			(*fn)(ecsql::SQLValue(argv[I]).get<Args>()...);
		}
		else {
			RetType result = (*fn)(ecsql::SQLValue(argv[I]).get<Args>()...);
			set_function_result(ctx, result);
		}
	}

	template<typename RetValue, typename... Args>
	void sql_function_entrypoint(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
		sql_function_entrypoint<RetValue, Args...>(ctx, argc, argv, std::index_sequence_for<Args...>());
	}
}

template<typename RetValue, typename... Args>
void register_sql_function(sqlite3 *db, RetValue (*fn)(Args...), const char *name, int eTextRep) {
	sqlite3_create_function(db, name, sizeof...(Args), eTextRep, (void *) fn, detail::sql_function_entrypoint<RetValue, Args...>, nullptr, nullptr);
}

}
