#pragma once

#include <functional>
#include <string>

struct sqlite3;

namespace ecsql {

class SQLRow;
class PreparedSQL;

class System {
public:
	System(const std::string& name, const std::string& sql, std::function<void(PreparedSQL&)> implementation);
	System(const std::string& name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation);

	void operator()();
	
	void prepare(sqlite3 *db);

	std::string name;
	std::vector<std::string> sql;
	std::vector<PreparedSQL> prepared_sql;
	std::function<void(std::vector<PreparedSQL>&)> implementation;
};

}
