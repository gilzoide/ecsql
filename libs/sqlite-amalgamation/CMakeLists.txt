add_library(sqlite3 "sqlite3.c")
set_source_files_properties("sqlite3.c"
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
target_include_directories(sqlite3 PUBLIC .)
target_compile_definitions(sqlite3 PUBLIC "SQLITE_ENABLE_PREUPDATE_HOOK")
