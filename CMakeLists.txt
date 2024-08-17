cmake_minimum_required(VERSION 3.5)

project(ecsql C CXX)

add_subdirectory(libs)

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})
target_compile_definitions(ecsql PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

file(GLOB_RECURSE textures "src/textures/*.png")
file(COPY ${textures}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures
)

target_link_libraries(ecsql libs)
set_target_properties(ecsql PROPERTIES CXX_STANDARD 20)
