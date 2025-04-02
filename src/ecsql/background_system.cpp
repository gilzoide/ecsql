#include <tracy/Tracy.hpp>

#include "background_system.hpp"

namespace ecsql {

BackgroundSystem::BackgroundSystem(std::string_view name, std::function<void()> implementation, bool join_before_new_frame)
	: name(name)
	, implementation(implementation)
	, join_before_new_frame(join_before_new_frame)
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

bool BackgroundSystem::should_join_before_new_frame() const {
	return join_before_new_frame;
}

}
