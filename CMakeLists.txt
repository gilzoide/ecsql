cmake_minimum_required(VERSION 3.18)

project(ecsql C CXX)

# Add "Asan" build type
include(tools/asan.cmake)

add_subdirectory(libs)

include(tools/file2c/file2c.cmake)
add_file2c(
  world_schema
  INPUT "src/ecsql/world_schema.sql"
  TEXT
)
add_file2c(
  game_schema
  INPUT "src/game_schema.sql"
  TEXT
)
add_file2c(
  lua_globals
  INPUT "src/scripting/lua_globals.lua"
  TEXT
)

file(GLOB_RECURSE src CONFIGURE_DEPENDS "src/*.cpp")
add_executable(ecsql ${src})
target_link_libraries(ecsql libs game_schema world_schema lua_globals)
target_compile_features(ecsql PUBLIC cxx_std_20)
target_compile_options(ecsql PRIVATE "-fbracket-depth=512")
# define `DEBUG` symbol on CMake Debug builds
target_compile_definitions(ecsql PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

# Emscripten / Web build support
if (EMSCRIPTEN)
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(assertionsopt "-sASSERTIONS")
  endif ()
  target_link_options(ecsql PRIVATE
    -sALLOW_MEMORY_GROWTH
    ${assertionsopt}
    --embed-file assets.zip
    --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/src/emscripten_shell.html"
  )
  set_target_properties(ecsql PROPERTIES
    OUTPUT_NAME "index"
    SUFFIX ".html"
    LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/emscripten_shell.html"
  )
endif ()


# Pack all assets in the assets.zip file
find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS assets/**)
set(pack_assets_script "tools/pack_assets.py")
add_custom_command(
  OUTPUT assets.zip
  COMMAND ${Python3_EXECUTABLE} ${pack_assets_script} assets "${CMAKE_BINARY_DIR}/assets.zip"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  DEPENDS ${asset_files} ${pack_assets_script}
)
add_custom_target(assets_zip DEPENDS assets.zip)
add_dependencies(ecsql assets_zip)
