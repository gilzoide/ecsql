#pragma once

#include <memory>
#include <span>
#include <string_view>

#include <sqlite3.h>

#include "sql_base_row.hpp"
#include "sql_value.hpp"

namespace ecsql {

struct SQLHookRow : public SQLBaseRow {
	SQLHookRow(sqlite3 *db, sqlite3_int64 rowid, bool use_new_row);

	sqlite3_int64 get_rowid() const;

	int column_count() const override;

	SQLValue column_value(int index) const;
	int column_type(int index) const override;

	bool column_bool(int index) const override;
	int column_int(int index) const override;
	sqlite3_int64 column_int64(int index) const override;
	double column_double(int index) const override;
	std::string_view column_text(int index) const override;
	std::span<const uint8_t> column_blob(int index) const override;

protected:
	sqlite3 *db;
	sqlite3_int64 rowid;
	bool use_new_row;
};

}
