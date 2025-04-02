#include <string>
#include <string_view>

#include "static_linked_list.hpp"

namespace ecsql {

class AdditionalSQL {
public:
	AdditionalSQL(std::string_view sql);

	const std::string& get_sql() const;

private:
	std::string sql;

	STATIC_LINKED_LIST_DEFINE(AdditionalSQL);
};

}
