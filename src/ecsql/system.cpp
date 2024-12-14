#include "prepared_sql.hpp"
#include "system.hpp"

namespace ecsql {

System::System(std::string_view name, std::function<void()> implementation)
	: System(name, {}, [=](World& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(); })
{
}

System::System(std::string_view name, std::function<void(World&)> implementation)
	: System(name, {}, [=](World& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(world); })
{
}

System::System(std::string_view name, const std::string& sql, std::function<void(PreparedSQL&)> implementation)
	: System(name, { sql }, [=](World& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(prepared_sqls[0]); })
{
}

System::System(std::string_view name, const std::string& sql, std::function<void(World&, PreparedSQL&)> implementation)
	: System(name, { sql }, [=](World& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(world, prepared_sqls[0]); })
{
}

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation)
	: System(name, sql, [=](World& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(prepared_sqls); })
{
}

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(World&, std::vector<PreparedSQL>&)> implementation)
	: name(name)
	, sql(sql)
	, prepared_sql()
	, implementation(implementation)
{
}

void System::operator()(World& world) {
	ZoneScoped;
	ZoneName(name.c_str(), name.size());
	implementation(world, prepared_sql);
}

void System::prepare(sqlite3 *db) {
	for (int i = prepared_sql.size(); i < sql.size(); i++) {
		prepared_sql.emplace_back(db, sql[i], true);
	}
}

}
