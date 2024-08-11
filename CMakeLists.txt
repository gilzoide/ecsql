cmake_minimum_required(VERSION 3.5)

project(ecsql C CXX)

add_subdirectory(libs)

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})

target_link_libraries(ecsql libs)
set_target_properties(ecsql PROPERTIES CXX_STANDARD 20)
