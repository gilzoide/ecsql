#include "hook_system.hpp"

namespace ecsql {

void HookSystem::operator()(HookType hook, SQLHookRow& old_row, SQLHookRow& new_row) const {
	implementation(hook, old_row, new_row);
}

}
