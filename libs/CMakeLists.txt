add_library(libs STATIC empty.c)

# Raylib
add_subdirectory(raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings
target_link_libraries(libs PUBLIC raylib)

# Reflect
target_include_directories(libs PUBLIC reflect)
