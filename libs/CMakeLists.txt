add_library(libs INTERFACE)

# box2d
add_subdirectory(box2d)
target_link_libraries(libs INTERFACE box2d)

# cdedent
add_subdirectory(cdedent)
target_link_libraries(libs INTERFACE cdedent)
target_compile_features(cdedent PRIVATE cxx_std_17)

# Dispatch Queue
add_subdirectory(dispatch-queue)
target_link_libraries(libs INTERFACE dispatch_queue)

# Flyweight.hpp
add_subdirectory(flyweight.hpp)
target_link_libraries(libs INTERFACE flyweight.hpp)

# idbvfs
if (EMSCRIPTEN)
  add_subdirectory(idbvfs)
  target_link_libraries(libs INTERFACE idbvfs)
endif ()

# Lua
add_library(lua STATIC "onelua.cpp")
target_compile_definitions(lua PRIVATE MAKE_LIB)
target_include_directories(lua PUBLIC lua)
target_link_libraries(libs INTERFACE lua)

# PhysFS
option(PHYSFS_BUILD_SHARED "Build shared library" OFF)
option(PHYSFS_BUILD_TEST "Build stdio test program." OFF)
option(PHYSFS_BUILD_DOCS "Build doxygen based documentation" OFF)
option(PHYSFS_ARCHIVE_7Z "Enable 7zip support" OFF)
option(PHYSFS_ARCHIVE_GRP "Enable Build Engine GRP support" OFF)
option(PHYSFS_ARCHIVE_WAD "Enable Doom WAD support" OFF)
option(PHYSFS_ARCHIVE_HOG "Enable Descent I/II HOG support" OFF)
option(PHYSFS_ARCHIVE_MVL "Enable Descent I/II MVL support" OFF)
option(PHYSFS_ARCHIVE_QPAK "Enable Quake I/II QPAK support" OFF)
option(PHYSFS_ARCHIVE_SLB "Enable I-War / Independence War SLB support" OFF)
option(PHYSFS_ARCHIVE_ISO9660 "Enable ISO9660 support" OFF)
option(PHYSFS_ARCHIVE_VDF "Enable Gothic I/II VDF archive support" OFF)
add_subdirectory(physfs)
target_link_libraries(libs INTERFACE PhysFS::PhysFS-static)

# PhysFS streambuf
add_subdirectory(physfs_streambuf)
target_link_libraries(libs INTERFACE physfs_streambuf)

# Raylib
add_subdirectory(raylib)
target_compile_options(raylib PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE raylib)

# Raylib Basis Universal
option(BASISD_SUPPORT_KTX2 "Enable KTX2 support as well as Zstd/miniz usage which is only required for UASTC supercompression in KTX2 files" OFF)
option(BASISD_SUPPORT_KTX2_ZSTD "Enable KTX2 UASTC Zstd supercompression support " OFF)
add_subdirectory(raylib-basis-universal)
target_link_libraries(libs INTERFACE raylib_basis_universal)

# Reflect
target_include_directories(libs INTERFACE reflect)

# Sol2 v3.0
set(SOL2_LUA_VERSION "5.4.7" CACHE STRING "")
add_subdirectory(sol2)
target_compile_definitions(sol2
  INTERFACE
    SOL_USING_CXX_LUA=1
    SOL_EXCEPTIONS_SAFE_PROPAGATION=1
)
target_link_libraries(libs INTERFACE sol2)

# SQLite 3
add_subdirectory(sqlite-amalgamation)
target_link_libraries(libs INTERFACE sqlite3)

# Tracy
if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
  option(TRACY_ENABLE "Enable profiling" OFF)
endif ()
add_subdirectory(tracy)
target_compile_options(TracyClient PRIVATE "-w")  # disable warnings
target_link_libraries(libs INTERFACE TracyClient)

# Yoga
add_subdirectory(yoga)
target_link_libraries(libs INTERFACE yogacore)
