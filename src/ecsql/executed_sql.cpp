#include <iostream>

#include <tracy/Tracy.hpp>

#include "executed_sql.hpp"

namespace ecsql {

ExecutedSQL::ExecutedSQL(std::shared_ptr<sqlite3_stmt> stmt)
	: stmt(stmt)
{
	++RowIterator(stmt);
}

ExecutedSQL::RowIterator ExecutedSQL::begin() {
	if (sqlite3_stmt_busy(stmt.get())) {
		return RowIterator(stmt);
	}
	else {
		return RowIterator();
	}
}

ExecutedSQL::RowIterator ExecutedSQL::end() {
	return RowIterator();
}

// RowIterator
ExecutedSQL::RowIterator::RowIterator(std::shared_ptr<sqlite3_stmt> stmt)
	: stmt(stmt)
{
}

ExecutedSQL::RowIterator& ExecutedSQL::RowIterator::operator++() {
	ZoneScopedN("step SQL");
	ZoneTextF("%s", sqlite3_sql(stmt.get()));
	switch (sqlite3_step(stmt.get())) {
		case SQLITE_ROW:
			break;

		default:
			std::cerr << sqlite3_errmsg(sqlite3_db_handle(stmt.get())) << std::endl;
			// fallthrough
		case SQLITE_DONE:
			stmt = nullptr;
			break;
	}
	return *this;
}

ExecutedSQL::RowIterator ExecutedSQL::RowIterator::operator++(int _) {
	auto retval = *this;
	++(*this);
	return retval;
}

SQLRow ExecutedSQL::RowIterator::row() const {
	return stmt;
}

SQLRow ExecutedSQL::RowIterator::operator*() const {
	return stmt;
}

ExecutedSQL::RowIterator::operator bool() const {
	return stmt.get();
}

bool ExecutedSQL::RowIterator::operator==(RowIterator other) const {
	return stmt == other.stmt;
}

bool ExecutedSQL::RowIterator::operator!=(RowIterator other) const {
	return stmt != other.stmt;
}

}
