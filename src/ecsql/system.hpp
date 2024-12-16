#pragma once

#include <functional>
#include <string>
#include <string_view>

#include <tracy/Tracy.hpp>

#include "prepared_sql.hpp"

struct sqlite3;

namespace ecsql {

class World;
class PreparedSQL;
class SQLRow;

class System {
public:
	System(std::string_view name, std::function<void()> implementation);
	System(std::string_view name, std::function<void(World&)> implementation);
	System(std::string_view name, const std::string& sql, std::function<void(PreparedSQL&)> implementation);
	System(std::string_view name, const std::string& sql, std::function<void(World&, PreparedSQL&)> implementation);
	System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation);
	System(std::string_view name, const std::vector<std::string>& sql, std::function<void(World&, std::vector<PreparedSQL>&)> implementation);

	void operator()(World& world, std::vector<PreparedSQL>& prepared_sql) const;

	void prepare(sqlite3 *db, std::vector<PreparedSQL>& prepared_sql) const;

	std::string name;
	std::vector<std::string> sql;
	std::function<void(World&, std::vector<PreparedSQL>&)> implementation;
};

}
