#pragma once

#include <string>
#include <string_view>
#include <span>
#include <vector>

#include <reflect>
#include <sqlite3.h>

#include "is_optional.hpp"

namespace ecsql {

struct SQLBaseRow {
	virtual ~SQLBaseRow() = default;

	virtual int column_count() const = 0;

	virtual int column_type(int index) const = 0;
	bool column_is_null(int index) const {
		return column_type(index) == SQLITE_NULL;
	}

	virtual bool column_bool(int index) const = 0;
	virtual int column_int(int index) const = 0;
	virtual sqlite3_int64 column_int64(int index) const = 0;
	virtual double column_double(int index) const = 0;
	virtual std::string_view column_text(int index) const = 0;
	virtual std::span<const uint8_t> column_blob(int index) const = 0;

	template<typename... Types> auto get(int index = 0) const {
		if constexpr (sizeof...(Types) == 1) {
			return get_advance<Types...>(index);
		}
		else {
			std::tuple<Types...> tuple;
			[&]<std::size_t... I> (std::index_sequence<I...>) {
				((std::get<I>(tuple) = get_advance<std::remove_cvref_t<decltype(std::get<I>(tuple))>>(index)), ...);
			} (std::index_sequence_for<Types...>());
			return tuple;
		}
	}

protected:
	template<typename T> T get_advance(int& index) const
	requires is_optional<T>
	{
		if (column_is_null(index)) {
			// make sure to advance the correct amount of columns
			get_advance<typename T::value_type>(index);
			return std::nullopt;
		}
		else {
			return get_advance<typename T::value_type>(index);
		}
	}

	template<typename T> T get_advance(int& index) const
	requires (not is_optional<T>) {
		T value;
		reflect::for_each<T>([&](auto I) {
			auto&& field = reflect::get<I>(value);
			field = get_advance<std::remove_cvref_t<decltype(field)>>(index);
		});
		return value;
	}

	template<> bool get_advance(int& index) const {
		return column_bool(index++);
	}

	template<> char get_advance(int& index) const {
		return column_int(index++);
	}
	template<> unsigned char get_advance(int& index) const {
		return column_int(index++);
	}

	template<> short get_advance(int& index) const {
		return column_int(index++);
	}
	template<> unsigned short get_advance(int& index) const {
		return column_int(index++);
	}

	template<> int get_advance(int& index) const {
		return column_int(index++);
	}
	template<> unsigned int get_advance(int& index) const {
		return column_int(index++);
	}

	template<> long get_advance(int& index) const {
		return column_int64(index++);
	}
	template<> unsigned long get_advance(int& index) const {
		return column_int64(index++);
	}

	template<> long long get_advance(int& index) const {
		return column_int64(index++);
	}
	template<> unsigned long long get_advance(int& index) const {
		return column_int64(index++);
	}

	template<> float get_advance(int& index) const {
		return column_double(index++);
	}

	template<> double get_advance(int& index) const {
		return column_double(index++);
	}

	template<> const unsigned char *get_advance(int& index) const {
		return (const unsigned char *) column_text(index++).data();
	}

	template<> const char *get_advance(int& index) const {
		return (const char *) column_text(index++).data();
	}

	template<> std::string get_advance(int& index) const {
		std::string_view text = column_text(index++);
		return std::string(text);
	}

	template<> std::string_view get_advance(int& index) const {
		return column_text(index++);
	}

	template<> std::span<const uint8_t> get_advance(int& index) const {
		return column_blob(index++);
	}

	template<> std::vector<uint8_t> get_advance(int& index) const {
		std::span<const uint8_t> span = column_blob(index++);
		return std::vector<uint8_t>(span.begin(), span.end());
	}
};

}
