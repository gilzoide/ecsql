#include "memory.hpp"

lua_Alloc lua_allocf = nullptr;

#ifdef TRACY_ENABLE
#include <cstdlib>

#include <box2d/box2d.h>
#include <physfs.h>
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

// Box2D memory allocator
const char *BOX2D_MEMORY_ZONE_NAME = "box2d";
static void *box2d_alloc(unsigned int size, int alignment) {
	void *ptr = std::aligned_alloc(alignment, size);
	TracyAllocN(ptr, size, BOX2D_MEMORY_ZONE_NAME);
	return ptr;
}

static void box2d_free(void *ptr) {
	TracyFreeN(ptr, BOX2D_MEMORY_ZONE_NAME);
	free(ptr);
}

static void configure_box2d_allocator() {
	b2SetAllocator(box2d_alloc, box2d_free);
}

// Lua memory allocator
const char *LUA_MEMORY_ZONE_NAME = "lua";
void *lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
	if (nsize == 0) {
		if (ptr) {
			TracyFreeN(ptr, LUA_MEMORY_ZONE_NAME);
			free(ptr);
		}
		return nullptr;
	}
	else {
		if (ptr) {
			TracyFreeN(ptr, LUA_MEMORY_ZONE_NAME);
		}
		ptr = realloc(ptr, nsize);
		TracyAllocN(ptr, nsize, LUA_MEMORY_ZONE_NAME);
		return ptr;
	}
}

// SQLite memory allocations
const char *SQLITE_MEMORY_ZONE_NAME = "sqlite3";
static sqlite3_mem_methods default_sqlite_mem_methods;

static void *sqlite_xMalloc(int size) {
	void *ptr = default_sqlite_mem_methods.xMalloc(size);
	TracyAllocN(ptr, size, SQLITE_MEMORY_ZONE_NAME);
	return ptr;
}

static void sqlite_xFree(void *ptr) {
	TracyFreeN(ptr, SQLITE_MEMORY_ZONE_NAME);
	default_sqlite_mem_methods.xFree(ptr);
}

static void *sqlite_xRealloc(void *ptr, int size) {
	TracyFreeN(ptr, SQLITE_MEMORY_ZONE_NAME);
	ptr = default_sqlite_mem_methods.xRealloc(ptr, size);
	TracyAllocN(ptr, size, SQLITE_MEMORY_ZONE_NAME);
	return ptr;
}

static void configure_sqlite_memory_methods() {
	sqlite3_config(SQLITE_CONFIG_GETMALLOC, &default_sqlite_mem_methods);
	sqlite3_mem_methods mem_methods = {
		sqlite_xMalloc,
		sqlite_xFree,
		sqlite_xRealloc,
		default_sqlite_mem_methods.xSize,
		default_sqlite_mem_methods.xRoundup,
		default_sqlite_mem_methods.xInit,
		default_sqlite_mem_methods.xShutdown,
		default_sqlite_mem_methods.pAppData,
	};
	sqlite3_config(SQLITE_CONFIG_MALLOC, &mem_methods);
}

// PhysFS Allocator
const char *PHYSFS_MEMORY_ZONE_NAME = "PhysFS";
static void *physfs_malloc(PHYSFS_uint64 size) {
	void *ptr = malloc(size);
	TracyAllocN(ptr, size, PHYSFS_MEMORY_ZONE_NAME);
	return ptr;
}

static void physfs_free(void *ptr) {
	TracyFreeN(ptr, PHYSFS_MEMORY_ZONE_NAME);
	free(ptr);
}

static void *physfs_realloc(void *ptr, PHYSFS_uint64 size) {
	TracyFreeN(ptr, PHYSFS_MEMORY_ZONE_NAME);
	ptr = realloc(ptr, size);
	TracyAllocN(ptr, size, PHYSFS_MEMORY_ZONE_NAME);
	return ptr;
}

static void configure_physfs_allocator() {
	static PHYSFS_Allocator physfs_allocator = {
		nullptr,
		nullptr,
		physfs_malloc,
		physfs_realloc,
		physfs_free,
	};
	PHYSFS_setAllocator(&physfs_allocator);
}
#endif // TRACY_ENABLE


void configure_memory_hooks() {
#ifdef TRACY_ENABLE
	configure_box2d_allocator();
	configure_sqlite_memory_methods();
	configure_physfs_allocator();
	lua_allocf = lua_alloc;
#endif
}
