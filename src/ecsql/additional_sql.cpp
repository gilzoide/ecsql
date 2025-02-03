#include "additional_sql.hpp"

namespace ecsql {

AdditionalSQL::AdditionalSQL(std::string_view sql)
	: sql(sql)
{
	STATIC_LINKED_LIST_INSERT();
}

const std::string& AdditionalSQL::get_sql() const {
	return sql;
}

}
