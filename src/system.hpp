#pragma once

#include "SQLRow.hpp"
#include <functional>
#include <string>

namespace ecsql {

class SQLRow;

class System {
public:
	System(const std::string& name, const std::string& sql, std::function<void(PreparedSQL&)> implementation);
	System(const std::string& name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation);

	void operator()(sqlite3 *db);

	std::string name;
	std::vector<std::string> sql;
	std::vector<PreparedSQL> prepared_sql;
	std::function<void(std::vector<PreparedSQL>&)> implementation;
};

}
