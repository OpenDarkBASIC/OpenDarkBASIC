cmake_minimum_required (VERSION 3.21)

# Make CMake shut the hell up about DOWNLOAD_EXTRACT_TIMESTAMP TRUE
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24) 
    cmake_policy (SET CMP0135 NEW)
endif ()

# Default to a release build on non-Windows platforms
if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

# Global options that affect subprojects
option (ODB_TESTS "Enable/disable unit tests in all subprojects" ON)
option (ODB_EDITOR "Build the code editor" OFF)
set (ODB_VERSION "0.0.1" CACHE STRING "OpenDarkBASIC version")

project ("OpenDarkBASIC"
    LANGUAGES C
    VERSION ${ODB_VERSION})

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# Try to create the following directory structure in both build and install
# trees (see issue #11)
#
#    build/Debug/ or build/Release or ...
#    ├── dbp-sdk/
#    │   ├── plugins/
#    │   └── thirdparty-plugins/
#    ├── odb-sdk/
#    │   ├── plugins/
#    │   |   └── test-plugin.dll
#    |   └── thirdparty-plugins/
#    ├── include/
#    │   ├── odb-compiler/
#    │   └── odb-sdk/
#    ├── lib/
#    │   ├── odb-compiler.lib
#    │   └── odb-sdk.lib
#    ├── odbc
#    ├── odbc_tests
#    ├── odbi
#    ├── odb-sdk.dll
#    └── odb-compiler.dll
#
set (ODB_BUILD_BINDIR "${PROJECT_BINARY_DIR}/bin")
set (ODB_BUILD_LIBDIR "${ODB_BUILD_BINDIR}/lib")
set (ODB_BUILD_SDKDIR "${ODB_BUILD_BINDIR}/odb-sdk")

set (ODB_INSTALL_BINDIR ".")
set (ODB_INSTALL_LIBDIR "lib")
set (ODB_INSTALL_INCLUDEDIR "include")
set (ODB_INSTALL_SDKDIR "odb-sdk")

if (ODB_TESTS)
    set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    include (FetchContent)
    FetchContent_Declare (
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7)
    FetchContent_MakeAvailable (googletest)
    add_subdirectory ("odb-tests")
endif ()

add_subdirectory ("thirdparty/gperf-3.1")
add_subdirectory ("odb-sdk")
add_subdirectory ("odb-compiler")
add_subdirectory ("odb-cli")
add_subdirectory ("odb-resgen")
add_subdirectory ("odb-runtime")
add_subdirectory ("odb-plugins")
if (ODB_EDITOR)
    add_subdirectory ("odb-editor")
endif ()
#add_subdirectory ("odb-interpreter")
