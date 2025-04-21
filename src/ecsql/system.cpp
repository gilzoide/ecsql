#include <tracy/Tracy.hpp>

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

System::System(std::string_view name, const std::string& sql)
	: System(name, sql, [](PreparedSQL& prepared_sql) { prepared_sql(); })
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

System::System(std::string_view name, const std::vector<std::string>& sql)
	: System(name, sql, [](std::vector<PreparedSQL>& prepared_sqls) { for (auto& prepared_sql : prepared_sqls) prepared_sql(); })
{
}

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation)
	: System(name, sql, [=](World& world, std::vector<PreparedSQL>& prepared_sqls) { implementation(prepared_sqls); })
{
}

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(World&, std::vector<PreparedSQL>&)> implementation)
	: name(name)
	, sql(sql)
	, implementation(implementation)
{
}

void System::operator()(World& world, std::vector<PreparedSQL>& prepared_sql) const {
	ZoneScoped;
	ZoneName(name.c_str(), name.size());
	implementation(world, prepared_sql);
	{
		ZoneScopedN("reset busy SQL");
		for (PreparedSQL& s : prepared_sql) {
			if (s.busy()) {
				s.reset();
			}
		}
	}
}

void System::prepare(sqlite3 *db, std::vector<PreparedSQL>& prepared_sql) const {
	for (int i = prepared_sql.size(); i < sql.size(); i++) {
		prepared_sql.emplace_back(db, sql[i], true);
	}
}

}
