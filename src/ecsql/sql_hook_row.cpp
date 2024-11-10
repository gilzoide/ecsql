#include "sql_hook_row.hpp"
#include "sqlite3.h"

namespace ecsql {

SQLHookRow::SQLHookRow(sqlite3 *db, sqlite3_int64 rowid, bool use_new_row)
    : db(db)
	, rowid(rowid)
    , use_new_row(use_new_row)
{
}

sqlite3_int64 SQLHookRow::column_rowid() const {
	return rowid;
}

int SQLHookRow::column_count() const {
	return sqlite3_preupdate_count(db);
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

std::string_view SQLHookRow::column_text(int index) const {
	sqlite3_value *value = column_value(index);
	const unsigned char *text = sqlite3_value_text(value);
	int size = sqlite3_value_bytes(value);
    return std::string_view((const char *) text, size);
}

std::span<const uint8_t> SQLHookRow::column_blob(int index) const {
	sqlite3_value *value = column_value(index);
	const uint8_t *data = (const uint8_t *) sqlite3_value_blob(value);
	int size = sqlite3_value_bytes(value);
	return std::span<const uint8_t>(data, size);
}

bool SQLHookRow::column_is_null(int index) const {
    return sqlite3_value_type(column_value(index)) == SQLITE_NULL;
}

}
