#pragma once

#include <functional>
#include <string>

namespace ecsql {

class SQLRow;

class System {
public:
	System(const std::string& name, std::function<void(SQLRow&)> implementation);

	template<typename Fn, typename... Args>
	static System from_function(const std::string& name, Fn implementation) {
		return System(name, [implementation](SQLRow& row) {
			std::tuple<Args...> args;
			implementation(args);
		});
	}

	std::string name;
	std::function<void(SQLRow&)> implementation;
};

}
