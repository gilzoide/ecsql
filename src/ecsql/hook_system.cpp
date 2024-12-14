#include "hook_system.hpp"

namespace ecsql {

void HookSystem::operator()(HookType hook, SQLBaseRow& old_row, SQLBaseRow& new_row) const {
	implementation(hook, old_row, new_row);
}

}
