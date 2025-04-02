#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <sqlite3.h>

namespace ecsql {

struct SQLValue {
	SQLValue(sqlite3_value *value);

	int get_type() const;
	bool is_null() const;

	bool get_bool() const;
	int get_int() const;
	sqlite3_int64 get_int64() const;
	double get_double() const;
	std::string_view get_text() const;
	std::span<const uint8_t> get_blob() const;

protected:
	sqlite3_value *value;
};

}
