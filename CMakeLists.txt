cmake_minimum_required(VERSION 3.18)

project(ecsql C CXX)

add_subdirectory(libs)

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})
target_link_libraries(ecsql libs)
target_compile_features(ecsql PUBLIC cxx_std_20)
# define `DEBUG` symbol on CMake Debug builds
target_compile_definitions(ecsql PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

# Emscripten / Web build support
if (EMSCRIPTEN)
  target_link_options(ecsql PRIVATE
    -sALLOW_MEMORY_GROWTH
    --embed-file assets.zip
  )
  set_target_properties(ecsql PROPERTIES SUFFIX ".html")
endif ()


# Pack all assets in the assets.zip file
file(GLOB_RECURSE assets assets/**)
file(ARCHIVE_CREATE
  OUTPUT ${CMAKE_BINARY_DIR}/assets.zip
  PATHS ${assets}
  FORMAT zip
)
