#include "memory.hpp"

#ifdef TRACY_ENABLE
#include <sqlite3.h>
#include <tracy/Tracy.hpp>

// Global operator new/delete
void *operator new(std::size_t n) {
	void *ptr = malloc(n);
	TracyAlloc(ptr, n);
	return ptr;
}

void operator delete(void * p) noexcept {
	TracyFree(p);
	free(p);
}

// SQLite memory allocations
const char *SQLITE_MEMORY_ZONE_NAME = "sqlite3";
static sqlite3_mem_methods default_sqlite_mem_methods;

static void *xMalloc(int size) {
	void *ptr = default_sqlite_mem_methods.xMalloc(size);
	TracyAllocN(ptr, size, SQLITE_MEMORY_ZONE_NAME);
	return ptr;
}

static void xFree(void *ptr) {
	TracyFreeN(ptr, SQLITE_MEMORY_ZONE_NAME);
	default_sqlite_mem_methods.xFree(ptr);
}

static void *xRealloc(void *ptr, int size) {
	TracyFreeN(ptr, SQLITE_MEMORY_ZONE_NAME);
	ptr = default_sqlite_mem_methods.xRealloc(ptr, size);
	TracyAllocN(ptr, size, SQLITE_MEMORY_ZONE_NAME);
	return ptr;
}

void configure_sqlite_memory_methods() {
	sqlite3_config(SQLITE_CONFIG_GETMALLOC, &default_sqlite_mem_methods);
	sqlite3_mem_methods mem_methods = {
		xMalloc,
		xFree,
		xRealloc,
		default_sqlite_mem_methods.xSize,
		default_sqlite_mem_methods.xRoundup,
		default_sqlite_mem_methods.xInit,
		default_sqlite_mem_methods.xShutdown,
		default_sqlite_mem_methods.pAppData,
	};
	sqlite3_config(SQLITE_CONFIG_MALLOC, &mem_methods);
}
#else
void configure_sqlite_memory_methods() {
}
#endif
