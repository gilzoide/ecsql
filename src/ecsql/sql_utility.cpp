#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>

#include "sql_utility.hpp"

namespace ecsql {

void execute_sql_script(sqlite3 *db, const char *sql) {
	char *errmsg;
	int result = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
	std::unique_ptr<char, sqlite3_deleter> error_message(errmsg);
	if (result != SQLITE_OK) {
		std::string msg = error_message.get();
		msg += " @ '";
		msg += sql;
		msg += '\'';
		throw std::runtime_error(msg);
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
