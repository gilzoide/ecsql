add_library(libs INTERFACE)

# Flyweight.hpp
add_subdirectory(flyweight.hpp)
target_link_libraries(libs INTERFACE flyweight.hpp)

# Raylib
if (EMSCRIPTEN)
    set(PLATFORM Web)
endif ()
add_subdirectory(raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE raylib)

# Reflect
target_include_directories(libs INTERFACE reflect)

# SQLite 3
add_subdirectory(sqlite-amalgamation)
target_link_libraries(libs INTERFACE sqlite3)

# Tracy
add_subdirectory(tracy)
target_compile_options(TracyClient PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE TracyClient)
