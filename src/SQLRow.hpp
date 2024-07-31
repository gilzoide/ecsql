#pragma once

#include <sqlite3.h>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include "reflect.hpp"

namespace ecsql {

class PreparedSQL;

struct SQLRow {
    SQLRow(sqlite3_stmt *stmt);

    bool column_bool(int index) const;
    int column_int(int index) const;
    sqlite3_int64 column_int64(int index) const;
    double column_double(int index) const;
    const unsigned char *column_text(int index) const;

    template<typename... Types> auto get(int index) const {
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
    sqlite3_stmt *stmt;

    template<typename T> T get_advance(int& index) const {
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

    template<> uint8_t get_advance(int& index) const {
        return column_int(index++);
    }

    template<> uint16_t get_advance(int& index) const {
        return column_int(index++);
    }

    template<> int get_advance(int& index) const {
        return column_int(index++);
    }

    template<> sqlite3_int64 get_advance(int& index) const {
        return column_int64(index++);
    }
    
    template<> float get_advance(int& index) const {
        return column_double(index++);
    }
    
    template<> double get_advance(int& index) const {
        return column_double(index++);
    }
    
    template<> const unsigned char *get_advance(int& index) const {
        return column_text(index++);
    }
    
    template<> const char *get_advance(int& index) const {
        return (const char *) column_text(index++);
    }
    
    template<> std::string get_advance(int& index) const {
        return (const char *) column_text(index++);
    }
    
    template<> std::string_view get_advance(int& index) const {
        int size = sqlite3_column_bytes(stmt, index);
        const char *text = get_advance<const char *>(index);
        return std::string_view(text, size);
    }
};

}
