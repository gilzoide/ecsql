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
	for (int i = prepared_sql.size(); i < sql.size(); i++) {
		prepared_sql.emplace_back(db, sql[i], true);
	}
}

}
