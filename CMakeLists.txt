cmake_minimum_required (VERSION 3.21)

# Default to a release build on non-Windows platforms
if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

# Global options that affect subprojects
option (ODB_TESTS "Enable/disable unit tests in all subprojects" ON)
set (ODB_VERSION "0.0.1" CACHE STRING "OpenDarkBASIC version")

project ("OpenDarkBASIC"
    LANGUAGES
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
    set (INSTALL_GTEST OFF CACHE BOOL "")
    include (FetchContent)
    FetchContent_Declare (
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH MD5=c8340a482851ef6a3fe618a082304cfc
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable (googletest)
endif ()

add_subdirectory ("thirdparty/gperf-3.1")
add_subdirectory ("odb-sdk")
#add_subdirectory ("odb-compiler")
#add_subdirectory ("odb-interpreter")
#add_subdirectory ("odb-cli")
