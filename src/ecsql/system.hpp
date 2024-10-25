#pragma once

#include <functional>
#include <string>
#include <string_view>

#include <tracy/Tracy.hpp>

#include "prepared_sql.hpp"

struct sqlite3;

namespace ecsql {

class Ecsql;
class PreparedSQL;
class SQLRow;

class System {
public:
	System(std::string_view name, std::function<void()> implementation);
	System(std::string_view name, std::function<void(Ecsql&)> implementation);
	System(std::string_view name, const std::string& sql, std::function<void(PreparedSQL&)> implementation);
	System(std::string_view name, const std::string& sql, std::function<void(Ecsql&, PreparedSQL&)> implementation);
	System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation);
	System(std::string_view name, const std::vector<std::string>& sql, std::function<void(Ecsql&, std::vector<PreparedSQL>&)> implementation);

	void operator()(Ecsql& world);

	void prepare(sqlite3 *db);

	std::string name;
	std::vector<std::string> sql;
	std::vector<PreparedSQL> prepared_sql;
	std::function<void(Ecsql&, std::vector<PreparedSQL>&)> implementation;
};

}
