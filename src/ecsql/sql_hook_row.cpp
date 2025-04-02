#include "sql_hook_row.hpp"
#include "sqlite3.h"

namespace ecsql {

SQLHookRow::SQLHookRow(sqlite3 *db, sqlite3_int64 rowid, bool use_new_row)
	: db(db)
	, rowid(rowid)
	, use_new_row(use_new_row)
{
}

sqlite3_int64 SQLHookRow::get_rowid() const {
	return rowid;
}

int SQLHookRow::column_count() const {
	return sqlite3_preupdate_count(db);
}

SQLValue SQLHookRow::column_value(int index) const {
	sqlite3_value *value;
	if (use_new_row) {
		sqlite3_preupdate_new(db, index, &value);
	}
	else {
		sqlite3_preupdate_old(db, index, &value);
	}
	return value;
}

int SQLHookRow::column_type(int index) const {
	return column_value(index).get_type();
}

bool SQLHookRow::column_bool(int index) const {
	return column_value(index).get_bool();
}

int SQLHookRow::column_int(int index) const {
	return column_value(index).get_int();
}

sqlite3_int64 SQLHookRow::column_int64(int index) const {
	return column_value(index).get_int64();
}

double SQLHookRow::column_double(int index) const {
	return column_value(index).get_double();
}

std::string_view SQLHookRow::column_text(int index) const {
	return column_value(index).get_text();
}

std::span<const uint8_t> SQLHookRow::column_blob(int index) const {
	return column_value(index).get_blob();
}

}
