#pragma once

#include "entity.hpp"
#include "executed_sql.hpp"
#include "is_span.hpp"
#include "sql_row.hpp"

namespace ecsql {

class PreparedSQL {
public:
    PreparedSQL() = default;
    PreparedSQL(sqlite3 *db, std::string_view str);
    PreparedSQL(sqlite3 *db, std::string_view str, bool is_persistent);

    PreparedSQL& bind_null(int index);
    PreparedSQL& bind_bool(int index, bool value);
    PreparedSQL& bind_int(int index, int value);
    PreparedSQL& bind_int64(int index, sqlite3_int64 value);
    PreparedSQL& bind_double(int index, double value);
    PreparedSQL& bind_text(int index, const char *value, int length = -1, void(*dtor)(void*) = SQLITE_STATIC);
    PreparedSQL& bind_text(int index, std::string_view value, void(*dtor)(void*) = SQLITE_STATIC);
    PreparedSQL& bind_blob(int index, void *data, int length, void(*dtor)(void*) = SQLITE_STATIC);
	template<typename T>
    PreparedSQL& bind_blob(int index, std::span<T> value, void(*dtor)(void*) = SQLITE_STATIC) {
		return bind_blob(index, (void *) value.data(), value.size_bytes(), dtor);
	}

    template<typename... Types>
    PreparedSQL& bind(int index, Types&&... values) {
        (bind_advance(index, std::forward<Types>(values)), ...);
        return *this;
    }

    PreparedSQL& reset();
    
    template<typename... Types>
    ExecutedSQL operator()(Types&&... values) {
        reset().bind(1, std::forward<Types>(values)...);
        return { stmt };
    }

    std::shared_ptr<sqlite3_stmt> get_stmt() const;

private:
    std::shared_ptr<sqlite3_stmt> stmt;

    template<typename T> PreparedSQL& bind_advance(int& index, T value) {
		if constexpr (is_span<T>) {
			bind_blob(index++, value);
		}
		else {
			reflect::for_each<T>([&](auto I) {
				auto&& field = reflect::get<I>(value);
				bind_advance<std::remove_cvref_t<decltype(field)>>(index, field);
			});
		}
        return *this;
    }

    template<> PreparedSQL& bind_advance(int& index, std::nullptr_t value) {
        return bind_null(index++);
    }

    template<> PreparedSQL& bind_advance(int& index, bool value) {
        return bind_bool(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, char value) {
        return bind_int(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, unsigned char value) {
        return bind_int(index++, value);
    }
    
    template<> PreparedSQL& bind_advance(int& index, short value) {
        return bind_int(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, unsigned short value) {
        return bind_int(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, int value) {
        return bind_int(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, unsigned int value) {
        return bind_int(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, long value) {
        return bind_int64(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, unsigned long value) {
        return bind_int64(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, long long value) {
        return bind_int64(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, unsigned long long value) {
        return bind_int64(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, float value) {
        return bind_double(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, double value) {
        return bind_double(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, const char *value) {
        return bind_text(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, const std::string& value) {
        return bind_text(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, std::string_view value) {
        return bind_text(index++, value);
    }
    
    template<> PreparedSQL& bind_advance(int& index, Entity value) {
        return bind_int64(index++, value.id);
    }
};

}
