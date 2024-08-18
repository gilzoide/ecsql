#include <iostream>
#include <stdexcept>

#include <sqlite3.h>

#include "prepared_sql.hpp"

namespace ecsql {

static sqlite3_stmt *prepare_statement(sqlite3 *db, std::string_view str, bool is_persistent) {
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v3(db, str.data(), str.size(), is_persistent ? SQLITE_PREPARE_PERSISTENT : 0, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db));
    }
    return stmt;
}

PreparedSQL::PreparedSQL(sqlite3 *db, std::string_view str)
    : PreparedSQL(db, str, false)
{
}

PreparedSQL::PreparedSQL(sqlite3 *db, std::string_view str, bool is_persistent)
    : stmt(prepare_statement(db, str, is_persistent), sqlite3_finalize)
{
}

PreparedSQL& PreparedSQL::bind_null(int index) {
    sqlite3_bind_null(stmt.get(), index);
    return *this;
}
PreparedSQL& PreparedSQL::bind_bool(int index, bool value) {
    sqlite3_bind_int(stmt.get(), index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_int(int index, int value) {
    sqlite3_bind_int(stmt.get(), index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_int64(int index, sqlite3_int64 value) {
    sqlite3_bind_int64(stmt.get(), index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_double(int index, double value) {
    sqlite3_bind_double(stmt.get(), index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_text(int index, const char *value, int length, void(*dtor)(void*)) {
    sqlite3_bind_text(stmt.get(), index, value, length, dtor);
    return *this;
}
PreparedSQL& PreparedSQL::bind_text(int index, std::string_view value, void(*dtor)(void*)) {
    sqlite3_bind_text(stmt.get(), index, value.data(), value.length(), dtor);
    return *this;
}

PreparedSQL& PreparedSQL::reset() {
    sqlite3_reset(stmt.get());
    return *this;
}

int PreparedSQL::step() {
    return sqlite3_step(stmt.get());
}

SQLRow PreparedSQL::step_single() {
    return *begin();
}

PreparedSQL::RowIterator PreparedSQL::begin() {
    return ++PreparedSQL::RowIterator { stmt };
}

PreparedSQL::RowIterator PreparedSQL::end() {
    return PreparedSQL::RowIterator { nullptr };
}

// RowIterator
PreparedSQL::RowIterator& PreparedSQL::RowIterator::operator++() {
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

PreparedSQL::RowIterator PreparedSQL::RowIterator::operator++(int _) {
    auto retval = *this;
    ++(*this);
    return retval;
}

SQLRow PreparedSQL::RowIterator::operator*() const {
    return stmt;
}

SQLRow PreparedSQL::RowIterator::operator->() const {
    return stmt;
}

bool PreparedSQL::RowIterator::operator==(RowIterator other) const {
    return stmt == other.stmt;
}

bool PreparedSQL::RowIterator::operator!=(RowIterator other) const {
    return stmt != other.stmt;
}

}
