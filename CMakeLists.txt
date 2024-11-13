cmake_minimum_required(VERSION 3.18)

project(ecsql C CXX)

add_subdirectory(libs)

include(tools/file2c/file2c.cmake)
add_file2c(
  world_schema
  INPUT "src/ecsql/world_schema.sql"
  TEXT
)
add_file2c(
  final_schema
  INPUT "src/final_schema.sql"
  TEXT
)

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src})
target_link_libraries(ecsql libs world_schema final_schema)
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
find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE asset_files assets/**)
set(pack_assets_script "tools/pack_assets.py")
add_custom_command(
  OUTPUT assets.zip
  COMMAND ${Python3_EXECUTABLE} ${pack_assets_script} assets "${CMAKE_BINARY_DIR}/assets.zip"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  DEPENDS ${asset_files} ${pack_assets_script}
)
add_custom_target(assets_zip DEPENDS assets.zip)
add_dependencies(ecsql assets_zip)
