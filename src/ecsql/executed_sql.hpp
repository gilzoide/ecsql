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
	void reset();

	template<typename... Types> auto get(int index = 0, bool auto_reset = true) {
		auto it = begin();
		auto value = (*it).get<Types...>(index);
		if (auto_reset) {
			reset();
		}
		return value;
	}

protected:
	std::shared_ptr<sqlite3_stmt> stmt;
};

}
