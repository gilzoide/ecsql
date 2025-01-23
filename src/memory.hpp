#pragma once

#include <lua.h>

extern lua_Alloc lua_allocf;

void configure_memory_hooks();
