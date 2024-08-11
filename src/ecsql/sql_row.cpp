#include "sql_row.hpp"

namespace ecsql {

SQLRow::SQLRow(std::shared_ptr<sqlite3_stmt> stmt)
    : stmt(stmt)
{
}

bool SQLRow::column_bool(int index) const {
    return sqlite3_column_int(stmt.get(), index);
}

int SQLRow::column_int(int index) const {
    return sqlite3_column_int(stmt.get(), index);
}

sqlite3_int64 SQLRow::column_int64(int index) const {
    return sqlite3_column_int64(stmt.get(), index);
}

double SQLRow::column_double(int index) const {
    return sqlite3_column_double(stmt.get(), index);
}

const unsigned char *SQLRow::column_text(int index) const {
    return sqlite3_column_text(stmt.get(), index);
}

bool SQLRow::column_is_null(int index) const {
    return sqlite3_column_type(stmt.get(), index) == SQLITE_NULL;
}

}
