cmake_minimum_required (VERSION 3.13.0)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

project ("OpenDarkBASIC")

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

include (ConfigureMSVCRuntime)
set (MSVC_RUNTIME "shared")
configure_msvc_runtime()

# Each subproject uses ODB_PLATFORM to create their own platform defines.
# Makes sense to put this here.
if (WIN32)
    set (ODB_PLATFORM "WIN32")
elseif (APPLE)
    set (ODB_PLATFORM "MACOS")
elseif (UNIX)
    set (ODB_PLATFORM "LINUX")
endif ()

# Try to create the following directory structure in both build and install
# trees (see issue #11)
#
#    build/bin/
#    ├── dbp-sdk/
#    │   ├── plugins/
#    │   └── thirdparty-plugins/
#    ├── odb-sdk/
#    │   ├── plugins/
#    │   |   └── test-plugin.dll
#    |   └── thirdparty-plugins/
#    ├── include/
#    │   ├── odb-compiler
#    │   └── odb-sdk
#    ├── lib/
#    │   ├── odb-compiler.lib
#    │   └── odb-sdk.lib
#    ├── odbc
#    ├── odbc_tests
#    ├── odbi
#    ├── odb-sdk.dll
#    └── odb-compiler.dll
#
set (ODB_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
set (ODB_RUNTIME_DIR ${ODB_OUTPUT_DIR})
set (ODB_SDK_DIR "${ODB_OUTPUT_DIR}/odb-sdk")
if (WIN32)
    set (ODB_LIBRARY_DIR ${ODB_OUTPUT_DIR})
    set (ODB_SDK_LIBRARY_DIR ${ODB_OUTPUT_DIR})
else ()
    set (ODB_LIBRARY_DIR "${ODB_OUTPUT_DIR}/lib")
    set (ODB_SDK_LIBRARY_DIR "${ODB_SDK_DIR}/lib")
endif ()
set (ODB_ARCHIVE_DIR "${ODB_RUNTIME_DIR}/lib")
set (ODB_SDK_ARCHIVE_DIR "${ODB_SDK_DIR}/lib")

set (CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}")
set (CMAKE_INSTALL_LIBDIR "lib")
set (CMAKE_INSTALL_INCLUDEDIR "include")
set (CMAKE_INSTALL_ODBSDKDIR "odb-sdk")
set (CMAKE_INSTALL_ODBSDK_BINDIR ${CMAKE_INSTALL_BINDIR})
set (CMAKE_INSTALL_ODBSDK_LIBDIR "${CMAKE_INSTALL_ODBSDKDIR}/lib")
set (CMAKE_INSTALL_ODBSDK_INCLUDEDIR "${CMAKE_INSTALL_ODBSDKDIR}/include")

set (INSTALL_GTEST OFF CACHE BOOL "")

if (WIN32)
    # Disable warning class 'foo' needs to have dll-interface to be used by clients of class 'bar'. We will get linker
    # errors if this turns out to be a real problem.
    add_compile_options(/wd4251)
    # Disable 'This function or variable may be unsafe. Consider using * instead.'
    add_definitions(/D_CRT_SECURE_NO_WARNINGS)
endif ()

add_subdirectory ("thirdparty/googletest")
add_subdirectory ("odb-sdk")
add_subdirectory ("odb-compiler")
add_subdirectory ("odb-interpreter")
