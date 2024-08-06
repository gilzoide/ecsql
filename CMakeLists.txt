cmake_minimum_required(VERSION 3.5)

project(ecsql C CXX)

add_subdirectory(libs/raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings

file(GLOB_RECURSE src "src/*.cpp")
add_executable(ecsql ${src} "sqlite-amalgamation/sqlite3.c")
set_source_files_properties("sqlite-amalgamation/sqlite3.c"
    PROPERTIES COMPILE_FLAGS
    "-O3 \
        -DSQLITE_USE_URI \
        -DSQLITE_DQS=0 \
        -DSQLITE_DEFAULT_MEMSTATUS=0 \
        -DSQLITE_DEFAULT_WAL_SYNCHRONOUS=1 \
        -DSQLITE_LIKE_DOESNT_MATCH_BLOBS \
        -DSQLITE_MAX_EXPR_DEPTH=0 \
        -DSQLITE_OMIT_DECLTYPE \
        -DSQLITE_OMIT_DEPRECATED \
        -DSQLITE_OMIT_PROGRESS_CALLBACK \
        -DSQLITE_OMIT_SHARED_CACHE \
        -DSQLITE_USE_ALLOCA \
        -DSQLITE_ENABLE_RTREE \
        -DSQLITE_ENABLE_MATH_FUNCTIONS \
        -DHAVE_ISNAN \
        -DSQLITE_ENABLE_GEOPOLY \
    "
)

target_link_libraries(ecsql raylib sqlite3)
set_target_properties(ecsql PROPERTIES CXX_STANDARD 20)
