#include <iostream>

#include "executed_sql.hpp"

namespace ecsql {

ExecutedSQL::ExecutedSQL(std::shared_ptr<sqlite3_stmt> stmt)
    : stmt(stmt)
    , executed_once(false)
{
}

ExecutedSQL::~ExecutedSQL() {
    if (!executed_once) {
        begin();
    }
}

ExecutedSQL::RowIterator ExecutedSQL::begin() {
    executed_once = true;
    return ++ExecutedSQL::RowIterator { stmt };
}

ExecutedSQL::RowIterator ExecutedSQL::end() {
    return ExecutedSQL::RowIterator {};
}

// RowIterator
ExecutedSQL::RowIterator::RowIterator(std::shared_ptr<sqlite3_stmt> stmt)
    : stmt(stmt)
{
}

ExecutedSQL::RowIterator& ExecutedSQL::RowIterator::operator++() {
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

SQLRow ExecutedSQL::RowIterator::operator*() const {
    return stmt;
}

SQLRow ExecutedSQL::RowIterator::operator->() const {
    return stmt;
}

bool ExecutedSQL::RowIterator::operator==(RowIterator other) const {
    return stmt == other.stmt;
}

bool ExecutedSQL::RowIterator::operator!=(RowIterator other) const {
    return stmt != other.stmt;
}

}
