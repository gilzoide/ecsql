#pragma once

#include <string_view>

#include <sqlite3.h>

#include "SQLRow.hpp"

using namespace std;

namespace ecsql {

class PreparedSQL : public SQLRow {
public:
    PreparedSQL(sqlite3 *db, const string_view& str);
    PreparedSQL(sqlite3 *db, const string_view& str, bool is_persistent);

    PreparedSQL& bind_null(int index);
    PreparedSQL& bind_bool(int index, bool value);
    PreparedSQL& bind_int(int index, int value);
    PreparedSQL& bind_int64(int index, sqlite3_int64 value);
    PreparedSQL& bind_double(int index, double value);
    PreparedSQL& bind_text(int index, const char *value, int length = -1, void(*dtor)(void*) = SQLITE_STATIC);
    PreparedSQL& bind_text(int index, const string_view& value, void(*dtor)(void*) = SQLITE_STATIC);

    template<typename... Types>
    PreparedSQL& bind(int index, Types... values) {
        if constexpr (sizeof...(Types) == 1) {
            return bind_advance<Types...>(index, values...);
        }
        else {
            (bind_advance(index, values), ...);
            return *this;
        }
    }

    PreparedSQL& reset();
    int step();

    struct RowIterator {
        using value_type = SQLRow;

        RowIterator& operator++();
        RowIterator operator++(int _);
        
        SQLRow operator*() const;
        SQLRow operator->() const;

        bool operator==(RowIterator other) const;
        bool operator!=(RowIterator other) const;

        std::shared_ptr<sqlite3_stmt> stmt;
    };

    RowIterator begin();
    RowIterator end();

private:
    template<typename T> PreparedSQL& bind_advance(int& index, T value) {
        reflect::for_each<T>([&](auto I) {
            auto&& field = reflect::get<I>(value);
            bind_advance<std::remove_cvref_t<decltype(field)>>(index, field);
        });
        return *this;
    }

    template<> PreparedSQL& bind_advance(int& index, nullptr_t value) {
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

    template<> PreparedSQL& bind_advance(int& index, const std::string_view& value) {
        return bind_text(index++, value);
    }
};

}
