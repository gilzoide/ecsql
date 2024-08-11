#include "SQLRow.hpp"

namespace ecsql {

SQLRow::SQLRow(sqlite3_stmt *stmt)
    : stmt(stmt)
{
}

bool SQLRow::column_bool(int index) const {
    return sqlite3_column_int(stmt, index);
}

int SQLRow::column_int(int index) const {
    return sqlite3_column_int(stmt, index);
}

sqlite3_int64 SQLRow::column_int64(int index) const {
    return sqlite3_column_int64(stmt, index);
}

double SQLRow::column_double(int index) const {
    return sqlite3_column_double(stmt, index);
}

const unsigned char *SQLRow::column_text(int index) const {
    return sqlite3_column_text(stmt, index);
}

bool SQLRow::column_is_null(int index) const {
    return sqlite3_column_type(stmt, index) == SQLITE_NULL;
}

}
