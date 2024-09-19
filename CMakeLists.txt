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
set (ODB_VERSION "0.0.1" CACHE STRING "OpenDarkBASIC version")
option (ODB_UTIL "Build utility library. This is required by all other subprojects" ON)
option (ODB_UTIL_i386 "Build utility library 32-bit as well." ON)
option (ODB_COMPILER "Build the compiler library" ON)
option (ODB_CLI "Build the commandline interface program for the compiler" ON)
option (ODB_SDK "Build the OpenDarkBASIC SDK" ON)
option (DBP_SDK "Build the DarkBASIC Pro bindings. This is necessary to compile DBP executables, but the DBPro game engine still has to be supplied externally." ON)
option (ODB_EDITOR "Build the code editor" OFF)
option (ODB_TESTS "Enable/disable unit tests in all subprojects" ON)

project ("OpenDarkBASIC"
    LANGUAGES C
    VERSION ${ODB_VERSION})

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

include (TestVisibilityMacros)
test_visibility_macros (
    ODB_API_EXPORT
    ODB_API_IMPORT
    ODB_API_LOCAL)

# Determine platform and architecture
string (TOLOWER "${CMAKE_SYSTEM_NAME}" ODB_PLATFORM)
string (TOLOWER "${CMAKE_GENERATOR_PLATFORM}" GENERATOR_PLATFORM)
string (TOLOWER "${CMAKE_VS_PLATFORM_NAME}" VS_PLATFORM_NAME)
if (GENERATOR_PLATFORM MATCHES "win32" OR VS_PLATFORM_NAME MATCHES "win32")
    set (ODB_ARCH "i386")
elseif (GENERATOR_PLATFORM MATCHES "x64" OR VS_PLATFORM_NAME MATCHES "x64")
    set (ODB_ARCH "x86_64")
else ()
    message (FATAL_ERROR "Couldn't determine target architecture.\nCMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}\nCMAKE_GENERATOR_PLATFORM: ${CMAKE_GENERATOR_PLATFORM}\nCMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
endif ()

# Try to create the following directory structure in both build and install
# trees (see issue #11)
#
#    build-<debug/release>/bin/
#    ├ include/
#    │   └ odb-compiler/
#    ├ lib/
#    │   └ odb-compiler.lib
#    ├ target/
#    │ └ <arch>/
#    │   └ <platform>/
#    │     ├ dbp-sdk/
#    │     │ ├ plugins/
#    │     │ ├ thirdparty-plugins/
#    │     │ └ runtime/
#    │     ├ odb-sdk/
#    │     │ ├ plugins/
#    │     │ │ └ core-commands.dll
#    │     │ │ thirdparty-plugins/
#    │     │ └ runtime/
#    │     └ odb-util/
#    │       ├ include/
#    │       │ └ odb-sdk/
#    │       └ lib/
#    │         └ odb-sdk.lib
#    ├ odb-cli
#    ├ odb-tests
#    ├ odb-util.dll
#    └ odb-compiler.dll
#
set (ODB_BUILD_PREFIX "${PROJECT_BINARY_DIR}/bin" CACHE STRING "Where to output all build artifacts")
set (ODB_BUILD_BINDIR "${ODB_BUILD_PREFIX}/${ODB_ARCH}/${ODB_PLATFORM}/bin")
set (ODB_BUILD_LIBDIR "${ODB_BUILD_PREFIX}/${ODB_ARCH}/${ODB_PLATFORM}/lib")
set (ODB_BUILD_SDKDIR "${ODB_BUILD_PREFIX}/${ODB_ARCH}/${ODB_PLATFORM}/odb-sdk")
set (DBP_BUILD_SDKDIR "${ODB_BUILD_PREFIX}/${ODB_ARCH}/${ODB_PLATFORM}/dbp-sdk")

set (ODB_BUILD_HOST_BINDIR "${ODB_BUILD_PREFIX}/hosttools/bin")

set (ODB_INSTALL_BINDIR "${ODB_ARCH}/${ODB_PLATFORM}/bin")
set (ODB_INSTALL_LIBDIR "${ODB_ARCH}/${ODB_PLATFORM}/lib")
set (ODB_INSTALL_INCLUDEDIR "${ODB_ARCH}/${ODB_PLATFORM}/include")

set (ODB_INSTALL_HOST_BINDIR "hosttools/bin")

if (ODB_TESTS)
    set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    include (FetchContent)
    FetchContent_Declare (
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7)
    FetchContent_MakeAvailable (googletest)
    include (ODBTargetProperties)
    odb_target_properties (gmock
        PROPERTIES
            MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
    add_subdirectory ("odb-tests")
endif ()

if (ODB_UTIL)
    add_subdirectory ("odb-util")
    # Also compile a 32-bit version of the utility library
    if (ODB_UTIL_i386 AND ODB_ARCH MATCHES "x86_64")
        if (NOT EXISTS "${PROJECT_BINARY_DIR}/build_odb-util-i386")
            execute_process (COMMAND ${CMAKE_COMMAND}
                -A win32 -B "${PROJECT_BINARY_DIR}/build_odb-util-i386" -S "${PROJECT_SOURCE_DIR}"
                -DODB_TESTS=OFF -DODB_COMPILER=OFF -DODB_CLI=OFF -DODB_EDITOR=OFF -DDBP_SDK=OFF
                -DODB_UTIL=ON -DODB_SDK=ON
                -DODB_BUILD_PREFIX=${ODB_BUILD_PREFIX})
        endif ()
        add_custom_target (odb-util-i386 ALL
            COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}/build_odb-util-i386")
        add_dependencies (odb-util odb-util-i386)
    endif ()

    if (ODB_COMPILER)
        add_subdirectory ("thirdparty/gperf-3.1")
        add_subdirectory ("odb-compiler")

        if (ODB_CLI)
            add_subdirectory ("odb-cligen")
            add_subdirectory ("odb-cli")
        endif ()
    endif ()

    if (ODB_SDK)
        add_subdirectory ("odb-resgen")
        add_subdirectory ("odb-sdk")
    endif ()

    if (ODB_EDITOR)
        add_subdirectory ("odb-editor")
    endif ()
endif ()

if (DBP_SDK)
    if (ODB_ARCH MATCHES "x86_64")
        if (NOT EXISTS "${PROJECT_BINARY_DIR}/build_dbp-sdk-i386")
            execute_process (COMMAND ${CMAKE_COMMAND}
                -A win32 -B "${PROJECT_BINARY_DIR}/build_dbp-sdk-i386" -S "${PROJECT_SOURCE_DIR}"
                -DODB_TESTS=OFF -DODB_COMPILER=OFF -DODB_CLI=OFF -DODB_EDITOR=OFF -DODB_UTIL=ON -DODB_SDK=ON
                -DDBP_SDK=ON
                -DODB_BUILD_PREFIX=${ODB_BUILD_PREFIX})
        endif ()
        add_custom_target (dbp-sdk-i386 ALL
            COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}/build_dbp-sdk-i386")
        if (TARGET odb-cli)
            add_dependencies (odb-cli dbp-sdk-i386)
        endif ()
    else ()
        add_subdirectory ("dbp-sdk")
    endif ()
endif ()
