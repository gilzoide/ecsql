#include "component.hpp"

namespace ecsql {

Component::Component(const std::string& name, const std::vector<std::string>& fields)
	: name(name)
	, fields(fields)
{
}
Component::Component(const std::string& name, std::vector<std::string>&& fields)
	: name(name)
	, fields(fields)
{
}

}
