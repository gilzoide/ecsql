#include "prepared_sql.hpp"
#include "system.hpp"

namespace ecsql {

System::System(std::string_view name, std::function<void()> implementation)
	: System(name, {}, [=](Ecsql& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(); })
{
}

System::System(std::string_view name, std::function<void(Ecsql&)> implementation)
	: System(name, {}, [=](Ecsql& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(world); })
{
}

System::System(std::string_view name, const std::string& sql, std::function<void(PreparedSQL&)> implementation)
	: System(name, { sql }, [=](Ecsql& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(prepared_sqls[0]); })
{
}

System::System(std::string_view name, const std::string& sql, std::function<void(Ecsql&, PreparedSQL&)> implementation)
	: System(name, { sql }, [=](Ecsql& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(world, prepared_sqls[0]); })
{
}

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation)
	: System(name, { sql }, [=](Ecsql& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(prepared_sqls); })
{
}

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(Ecsql&, std::vector<PreparedSQL>&)> implementation)
	: name(name)
	, sql(sql)
	, prepared_sql()
	, implementation(implementation)
{
}

void System::operator()(Ecsql& world) {
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
