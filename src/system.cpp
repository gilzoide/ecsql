#include "system.hpp"

namespace ecsql {

System::System(const std::string& name, std::function<void(SQLRow&)> implementation)
	: name(name)
	, implementation(implementation)
{
}

}
