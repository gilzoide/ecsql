#pragma once

#include <functional>
#include <string>
#include <string_view>

#include <tracy/Tracy.hpp>

#include "prepared_sql.hpp"

struct sqlite3;

namespace ecsql {

class SQLRow;
class PreparedSQL;

class System {
public:
	System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation);

	template<typename Fn, typename... Sqls>
	System(std::string_view name, Fn&& f, Sqls&&... sqls)
		: name(name)
		, sql(std::vector<std::string> { std::forward<Sqls>(sqls)... })
		, implementation([=](std::vector<PreparedSQL>& prepared_sqls) {
			ZoneScoped;
			ZoneName(name.data(), name.size());
			[&]<std::size_t... I> (std::index_sequence<I...>) {
                f(prepared_sqls[I]...);
            } (std::index_sequence_for<Sqls...>());
		})
	{
	}

	void operator()();

	void prepare(sqlite3 *db);

	std::string name;
	std::vector<std::string> sql;
	std::vector<PreparedSQL> prepared_sql;
	std::function<void(std::vector<PreparedSQL>&)> implementation;
};

}
