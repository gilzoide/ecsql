add_library(libs INTERFACE)

# Raylib
add_subdirectory(raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE raylib)

# Reflect
target_include_directories(libs INTERFACE reflect)

# SQLite 3
add_subdirectory(sqlite-amalgamation)
target_link_libraries(libs INTERFACE sqlite3)
