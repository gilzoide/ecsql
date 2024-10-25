#include <memory>
#include <stdexcept>

#include "sql_script.hpp"

namespace ecsql {

struct sqlite3_deleter {
	void operator()(void *ptr) {
		sqlite3_free(ptr);
	}
};

void execute_sql_script(sqlite3 *db, const char *sql) {
	char *errmsg;
	int result = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
	std::unique_ptr<char, sqlite3_deleter> error_message(errmsg);
	if (result != SQLITE_OK) {
		throw std::runtime_error(error_message.get());
	}
}

}
