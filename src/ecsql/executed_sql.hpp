#pragma once

#include "sql_row.hpp"

namespace ecsql {

class ExecutedSQL {
public:
    ExecutedSQL(std::shared_ptr<sqlite3_stmt> stmt);
    ~ExecutedSQL();

    ExecutedSQL(const ExecutedSQL& stmt) = delete;
    ExecutedSQL& operator=(const ExecutedSQL& stmt) = delete;

    class RowIterator {
    public:
        using value_type = SQLRow;

        RowIterator() = default;
        RowIterator(std::shared_ptr<sqlite3_stmt> stmt);

        RowIterator& operator++();
        RowIterator operator++(int _);
        
        SQLRow operator*() const;
        SQLRow operator->() const;

        bool operator==(RowIterator other) const;
        bool operator!=(RowIterator other) const;

    protected:
        std::shared_ptr<sqlite3_stmt> stmt;
    };

    RowIterator begin();
    RowIterator end();

    template<typename... Types> auto get(int index = 0) {
        auto it = begin();
        auto result = (*it).get<Types...>(index);
        sqlite3_reset(stmt.get());
        return result;
    }
    
protected:
    std::shared_ptr<sqlite3_stmt> stmt;
    bool executed_once;
};

}
