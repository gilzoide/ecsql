#include "prepared_sql.hpp"
#include "system.hpp"

namespace ecsql {

System::System(const std::string& name, const std::string& sql, std::function<void(PreparedSQL&)> implementation)
	: System(name, { sql }, [implementation](std::vector<PreparedSQL>& sqls) { implementation(sqls[0]); })
{
}

System::System(const std::string& name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation)
	: name(name)
	, sql(sql)
	, prepared_sql()
	, implementation(implementation)
{
}

void System::operator()() {
	implementation(prepared_sql);
}

void System::prepare(sqlite3 *db) {
	while (prepared_sql.size() < sql.size()) {
		prepared_sql.emplace_back(db, sql[prepared_sql.size()], true);
	}
}

}
