cmake_minimum_required(VERSION 3.5)

project(ecsql C CXX)

add_subdirectory(libs)

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})
target_compile_definitions(ecsql PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

file(GLOB_RECURSE assets
    RELATIVE ${CMAKE_SOURCE_DIR}/assets
    assets/**
)
foreach(asset IN LISTS assets)
    configure_file(assets/${asset} ${CMAKE_BINARY_DIR}/assets/${asset} COPYONLY)
endforeach()

target_link_libraries(ecsql libs)
set_target_properties(ecsql PROPERTIES CXX_STANDARD 20)

# Emscripten / Web build support
if (EMSCRIPTEN)
    target_link_options(ecsql PRIVATE
        -sALLOW_MEMORY_GROWTH
        --embed-file textures
    )
    set_target_properties(ecsql PROPERTIES SUFFIX ".html")
endif ()
