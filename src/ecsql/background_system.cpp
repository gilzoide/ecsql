#include <tracy/Tracy.hpp>

#include "background_system.hpp"

namespace ecsql {

BackgroundSystem::BackgroundSystem(std::string_view name, std::function<void()> implementation)
	: name(name)
	, implementation(implementation)
{
}

void BackgroundSystem::operator()() {
	ZoneScoped;
	ZoneName(name.c_str(), name.size());
	implementation();
}

const std::string& BackgroundSystem::get_name() const {
	return name;
}

}
