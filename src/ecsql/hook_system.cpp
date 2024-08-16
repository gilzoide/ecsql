#include "hook_system.hpp"

namespace ecsql {

void HookSystem::operator()(SQLHookRow& row) {
    implementation(row);
}

}
