#pragma once

#include <functional>
#include <string>

struct sqlite3;

namespace ecsql {

class SQLRow;
class PreparedSQL;

class System {
public:
	System(std::string_view name, std::string_view sql, std::function<void(PreparedSQL&)> implementation);
	System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation);

	void operator()();

	void prepare(sqlite3 *db);

	std::string name;
	std::vector<std::string> sql;
	std::vector<PreparedSQL> prepared_sql;
	std::function<void(std::vector<PreparedSQL>&)> implementation;
};

}
