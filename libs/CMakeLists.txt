add_library(libs INTERFACE)

# cdedent
add_subdirectory(cdedent)
target_link_libraries(libs INTERFACE cdedent)
target_compile_features(cdedent PRIVATE cxx_std_17)

# Flyweight.hpp
add_subdirectory(flyweight.hpp)
target_link_libraries(libs INTERFACE flyweight.hpp)

# idbvfs
if (EMSCRIPTEN)
  add_subdirectory(idbvfs)
  target_link_libraries(libs INTERFACE idbvfs)
endif ()

# Lua
add_library(lua STATIC "lua/onelua.c")
set_source_files_properties("lua/onelua.c" PROPERTIES LANGUAGE CXX)
target_compile_definitions(lua PRIVATE MAKE_LIB)
target_include_directories(lua PUBLIC lua)
target_link_libraries(libs INTERFACE lua)

# PhysFS
option(PHYSFS_BUILD_SHARED "Build shared library" OFF)
option(PHYSFS_BUILD_TEST "Build stdio test program." OFF)
option(PHYSFS_BUILD_DOCS "Build doxygen based documentation" OFF)
add_subdirectory(physfs)
target_link_libraries(libs INTERFACE PhysFS::PhysFS-static)

# Raylib
if (EMSCRIPTEN)
  set(PLATFORM Web CACHE STRING "")
endif ()
add_subdirectory(raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE raylib)

# Reflect
target_include_directories(libs INTERFACE reflect)

# Sol2 v3.0
set(SOL2_LUA_VERSION "5.4.7")
add_subdirectory(sol2)
target_compile_definitions(sol2
  INTERFACE
    SOL_USING_CXX_LUA=1
    SOL_EXCEPTIONS_SAFE_PROPAGATION=1
)
target_link_libraries(libs INTERFACE sol2)

# SQLite 3
add_subdirectory(sqlite-amalgamation)
target_link_libraries(libs INTERFACE sqlite3)

# Tracy
if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
  option(TRACY_ENABLE "Enable profiling" OFF)
endif ()
add_subdirectory(tracy)
target_compile_options(TracyClient PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE TracyClient)

# Yoga
add_subdirectory(yoga)
target_link_libraries(libs INTERFACE yogacore)
