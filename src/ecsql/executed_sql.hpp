#pragma once

#include "sql_row.hpp"

namespace ecsql {

class ExecutedSQL {
public:
	ExecutedSQL(std::shared_ptr<sqlite3_stmt> stmt);

	class RowIterator {
	public:
		using value_type = SQLRow;

		RowIterator() = default;
		RowIterator(std::shared_ptr<sqlite3_stmt> stmt);

		RowIterator& operator++();
		RowIterator operator++(int _);

		SQLRow row() const;
		SQLRow operator*() const;
		operator bool() const;

		bool operator==(RowIterator other) const;
		bool operator!=(RowIterator other) const;

	protected:
		std::shared_ptr<sqlite3_stmt> stmt;
	};

	RowIterator begin();
	RowIterator end();

	template<typename... Types> auto get(int index = 0) {
		auto it = begin();
		return (*it).get<Types...>(index);
	}

protected:
	std::shared_ptr<sqlite3_stmt> stmt;
};

}
