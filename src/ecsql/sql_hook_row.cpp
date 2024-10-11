#include "sql_hook_row.hpp"
#include "sqlite3.h"

namespace ecsql {

SQLHookRow::SQLHookRow(sqlite3 *db, bool use_new_row)
    : db(db)
    , use_new_row(use_new_row)
{
}

sqlite3_value *SQLHookRow::column_value(int index) const {
    sqlite3_value *value;
    if (use_new_row) {
        sqlite3_preupdate_new(db, index, &value);
    }
    else {
        sqlite3_preupdate_old(db, index, &value);
    }
    return value;
}

bool SQLHookRow::column_bool(int index) const {
    return sqlite3_value_int(column_value(index));
}
int SQLHookRow::column_int(int index) const {
    return sqlite3_value_int(column_value(index));
}
sqlite3_int64 SQLHookRow::column_int64(int index) const {
    return sqlite3_value_int64(column_value(index));
}
double SQLHookRow::column_double(int index) const {
    return sqlite3_value_double(column_value(index));
}
const unsigned char *SQLHookRow::column_text(int index) const {
    return sqlite3_value_text(column_value(index));
}

bool SQLHookRow::column_is_null(int index) const {
    return sqlite3_value_type(column_value(index)) == SQLITE_NULL;
}

}
