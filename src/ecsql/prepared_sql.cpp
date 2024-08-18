#include <iostream>
#include <stdexcept>

#include <sqlite3.h>

#include "prepared_sql.hpp"

namespace ecsql {

static sqlite3_stmt *prepare_statement(sqlite3 *db, std::string_view str, bool is_persistent) {
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare_v3(db, str.data(), str.size(), is_persistent ? SQLITE_PREPARE_PERSISTENT : 0, &stmt, NULL);
    if (res != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db);
        error += " (\"";
        error += str;
        error += "\")";
        throw std::runtime_error(error);
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

std::shared_ptr<sqlite3_stmt> PreparedSQL::get_stmt() const {
    return stmt;
}

}
