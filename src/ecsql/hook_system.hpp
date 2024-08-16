#pragma once

#include <functional>
#include <string>
#include <string_view>

struct sqlite3;

namespace ecsql {

class SQLRow;
class SQLHookRow;
class PreparedSQL;

class HookSystem {
public:
    template<typename Fn>
	HookSystem(std::string_view component_name, Fn&& implementation)
        : component_name(component_name)
        , implementation([=](SQLHookRow& row) { implementation(row); })
    {
    }

	void operator()(SQLHookRow& row);

	std::string component_name;
	std::function<void(SQLHookRow&)> implementation;
};

}
