#include "prepared_sql.hpp"
#include "system.hpp"

namespace ecsql {

System::System(std::string_view name, const std::vector<std::string>& sql, std::function<void(std::vector<PreparedSQL>&)> implementation)
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
