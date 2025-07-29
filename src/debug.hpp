#pragma once

#if defined(DEBUG) && !defined(NDEBUG)

namespace ecsql {
	class World;
}

bool is_paused();
void run_debug_functionality(ecsql::World& world);


#endif
