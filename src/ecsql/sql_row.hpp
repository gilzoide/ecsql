#pragma once

#include <memory>
#include <span>
#include <string_view>

#include <sqlite3.h>

#include "sql_base_row.hpp"

namespace ecsql {

struct SQLRow : public SQLBaseRow {
	SQLRow() = default;
	SQLRow(std::shared_ptr<sqlite3_stmt> stmt);

	int column_count() const override;

	int column_type(int index) const override;

	bool column_bool(int index) const override;
	int column_int(int index) const override;
	sqlite3_int64 column_int64(int index) const override;
	double column_double(int index) const override;
	std::string_view column_text(int index) const override;
	std::span<const uint8_t> column_blob(int index) const override;

	operator bool() const;

protected:
	std::shared_ptr<sqlite3_stmt> stmt;
};

}
