#pragma once

#include "SQLRow.hpp"
#include <string_view>

#include <sqlite3.h>

using namespace std;

namespace ecsql {

class PreparedSQL : public SQLRow {
public:
    PreparedSQL(sqlite3 *db, const string_view& str);
    PreparedSQL(sqlite3 *db, const string_view& str, bool is_persistent);
    ~PreparedSQL();

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

private:
    template<typename T> PreparedSQL& bind_advance(int& index, T value) {
        reflect::for_each<T>([&](auto I) {
            auto& field = reflect::get<I>(value);
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

    template<> PreparedSQL& bind_advance(int& index, int8_t value) {
        return bind_int(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, uint8_t value) {
        return bind_int(index++, value);
    }
    
    template<> PreparedSQL& bind_advance(int& index, int16_t value) {
        return bind_int(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, uint16_t value) {
        return bind_int(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, int32_t value) {
        return bind_int(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, uint32_t value) {
        return bind_int(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, int64_t value) {
        return bind_int64(index++, value);
    }
    template<> PreparedSQL& bind_advance(int& index, uint64_t value) {
        return bind_int64(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, float value) {
        return bind_double(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, double value) {
        return bind_double(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, const char * value) {
        return bind_text(index++, value);
    }

    template<> PreparedSQL& bind_advance(int& index, const std::string_view& value) {
        return bind_text(index++, value);
    }
};

}
