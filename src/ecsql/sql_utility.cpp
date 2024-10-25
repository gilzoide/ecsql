#include <cctype>
#include <memory>
#include <stdexcept>

#include "sql_utility.hpp"

namespace ecsql {

void execute_sql_script(sqlite3 *db, const char *sql) {
	char *errmsg;
	int result = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
	std::unique_ptr<char, sqlite3_deleter> error_message(errmsg);
	if (result != SQLITE_OK) {
		throw std::runtime_error(error_message.get());
	}
}

std::string_view extract_identifier(std::string_view field) {
	auto it = field.cbegin();
	while (it != field.cend() && isspace(*it)) {
		it++;
	}
	while (it != field.cend() && isalnum(*it)) {
		it++;
	}
	return std::string_view(field.cbegin(), it);
}

}
