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

    PreparedSQL& bind(int index, nullptr_t value);
    PreparedSQL& bind(int index, int value);
    PreparedSQL& bind(int index, sqlite3_int64 value);
    PreparedSQL& bind(int index, bool value);
    PreparedSQL& bind(int index, double value);
    PreparedSQL& bind(int index, const char *value, int length = -1, void(*dtor)(void*) = SQLITE_STATIC);
    PreparedSQL& bind(int index, const string_view& value, void(*dtor)(void*) = SQLITE_STATIC);

    template<typename... Types>
    PreparedSQL& bind_all(int index, Types... values) {
        [&]<std::size_t... I> (std::index_sequence<I...>) {
            (bind(index + I, values), ...);
        } (std::index_sequence_for<Types...>());
        return *this;
    }

    PreparedSQL& reset();
    int step();
};

}
