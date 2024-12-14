#include "sql_row.hpp"

namespace ecsql {

SQLRow::SQLRow(std::shared_ptr<sqlite3_stmt> stmt)
	: stmt(stmt)
{
}

int SQLRow::column_count() const {
	return sqlite3_column_count(stmt.get());
}

int SQLRow::column_type(int index) const {
	return sqlite3_column_type(stmt.get(), index);
}

bool SQLRow::column_bool(int index) const {
	return sqlite3_column_int(stmt.get(), index);
}

int SQLRow::column_int(int index) const {
	return sqlite3_column_int(stmt.get(), index);
}

sqlite3_int64 SQLRow::column_int64(int index) const {
	return sqlite3_column_int64(stmt.get(), index);
}

double SQLRow::column_double(int index) const {
	return sqlite3_column_double(stmt.get(), index);
}

std::string_view SQLRow::column_text(int index) const {
	const unsigned char *text = sqlite3_column_text(stmt.get(), index);
	int size = sqlite3_column_bytes(stmt.get(), index);
	return std::string_view((const char *) text, size);
}

std::span<const uint8_t> SQLRow::column_blob(int index) const {
	const uint8_t *data = (const uint8_t *) sqlite3_column_blob(stmt.get(), index);
	int size = sqlite3_column_bytes(stmt.get(), index);
	return std::span<const uint8_t>(data, size);
}

SQLRow::operator bool() const {
	return stmt != nullptr;
}

}
