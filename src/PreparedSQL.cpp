#include "PreparedSQL.hpp"

#include <sqlite3.h>
#include <stdexcept>

namespace ecsql {

PreparedSQL::PreparedSQL(sqlite3 *db, const string_view& str)
    : PreparedSQL(db, str, false)
{
}
PreparedSQL::PreparedSQL(sqlite3 *db, const string_view& str, bool is_persistent)
    : SQLRow(nullptr)
{
    int res = sqlite3_prepare_v3(db, str.data(), str.size(), is_persistent ? SQLITE_PREPARE_PERSISTENT : 0, &stmt, NULL);
    if (res != SQLITE_OK) {
        throw runtime_error(sqlite3_errmsg(db));
    }
}

PreparedSQL::~PreparedSQL() {
    sqlite3_finalize(stmt);
}

PreparedSQL& PreparedSQL::bind_null(int index) {
    sqlite3_bind_null(stmt, index);
    return *this;
}
PreparedSQL& PreparedSQL::bind_bool(int index, bool value) {
    sqlite3_bind_int(stmt, index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_int(int index, int value) {
    sqlite3_bind_int(stmt, index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_int64(int index, sqlite3_int64 value) {
    sqlite3_bind_int64(stmt, index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_double(int index, double value) {
    sqlite3_bind_double(stmt, index, value);
    return *this;
}
PreparedSQL& PreparedSQL::bind_text(int index, const char *value, int length, void(*dtor)(void*)) {
    sqlite3_bind_text(stmt, index, value, length, dtor);
    return *this;
}
PreparedSQL& PreparedSQL::bind_text(int index, const string_view& value, void(*dtor)(void*)) {
    sqlite3_bind_text(stmt, index, value.data(), value.length(), dtor);
    return *this;
}

PreparedSQL& PreparedSQL::reset() {
    sqlite3_reset(stmt);
    return *this;
}
int PreparedSQL::step() {
    return sqlite3_step(stmt);
}

}
