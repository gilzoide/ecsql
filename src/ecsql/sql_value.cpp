#include "sql_value.hpp"

namespace ecsql {

SQLValue::SQLValue(sqlite3_value *value)
	: value(value)
{
}

int SQLValue::get_type() const {
	return sqlite3_value_type(value);
}

bool SQLValue::is_null() const {
	return get_type() == SQLITE_NULL;
}

bool SQLValue::get_bool() const {
	return sqlite3_value_int(value);
}

int SQLValue::get_int() const {
	return sqlite3_value_int(value);
}

sqlite3_int64 SQLValue::get_int64() const {
	return sqlite3_value_int64(value);
}

double SQLValue::get_double() const {
	return sqlite3_value_double(value);
}

std::string_view SQLValue::get_text() const {
	const unsigned char *text = sqlite3_value_text(value);
	int size = sqlite3_value_bytes(value);
    return std::string_view((const char *) text, size);
}

std::span<const uint8_t> SQLValue::get_blob() const {
	const uint8_t *data = (const uint8_t *) sqlite3_value_blob(value);
	int size = sqlite3_value_bytes(value);
	return std::span<const uint8_t>(data, size);
}

}
