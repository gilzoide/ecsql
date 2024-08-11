cmake_minimum_required(VERSION 3.5)

project(ecsql C CXX)

add_subdirectory(sqlite-amalgamation)
add_subdirectory(libs/raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})

target_link_libraries(ecsql raylib sqlite3)
set_target_properties(ecsql PROPERTIES CXX_STANDARD 20)
