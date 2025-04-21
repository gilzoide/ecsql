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

	template<typename T> T get() const {
		static_assert(false, "Type is not supported!");
	}

	template<> bool get() const {
		return get_bool();
	}

	template<> char get() const {
		return get_int();
	}
	template<> unsigned char get() const {
		return get_int();
	}

	template<> short get() const {
		return get_int();
	}
	template<> unsigned short get() const {
		return get_int();
	}

	template<> int get() const {
		return get_int();
	}
	template<> unsigned int get() const {
		return get_int();
	}

	template<> long get() const {
		return get_int64();
	}
	template<> unsigned long get() const {
		return get_int64();
	}

	template<> long long get() const {
		return get_int64();
	}
	template<> unsigned long long get() const {
		return get_int64();
	}

	template<> float get() const {
		return get_double();
	}

	template<> double get() const {
		return get_double();
	}

	template<> const unsigned char *get() const {
		return (const unsigned char *) get_text().data();
	}

	template<> const char *get() const {
		return (const char *) get_text().data();
	}

	template<> std::string get() const {
		std::string_view text = get_text();
		return std::string(text);
	}

	template<> std::string_view get() const {
		return get_text();
	}

	template<> std::span<const uint8_t> get() const {
		return get_blob();
	}

	template<> std::vector<uint8_t> get() const {
		std::span<const uint8_t> span = get_blob();
		return std::vector<uint8_t>(span.begin(), span.end());
	}

protected:
	sqlite3_value *value;
};

}
