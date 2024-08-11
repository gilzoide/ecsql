cmake_minimum_required(VERSION 3.5)

project(ecsql C CXX)

add_subdirectory(libs)
add_subdirectory(sqlite-amalgamation)

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})

target_link_libraries(ecsql sqlite3 libs)
set_target_properties(ecsql PROPERTIES CXX_STANDARD 20)
