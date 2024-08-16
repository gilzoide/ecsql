#include "hook_system.hpp"

namespace ecsql {

void HookSystem::operator()(SQLHookRow& old_row, SQLHookRow& new_row) const {
    implementation(old_row, new_row);
}

}
