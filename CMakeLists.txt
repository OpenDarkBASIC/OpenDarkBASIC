cmake_minimum_required (VERSION 3.13.0)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

project ("OpenDarkBASIC")

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

include (ConfigureMSVCRuntime)
set (MSVC_RUNTIME "shared")
configure_msvc_runtime()

if (WIN32)
    set (ODB_PLATFORM "WIN32")
elseif (APPLE)
    set (ODB_PLATFORM "MACOS")
elseif (UNIX)
    set (ODB_PLATFORM "LINUX")
endif ()

add_subdirectory ("thirdparty/googletest")
add_subdirectory ("odb-sdk")
add_subdirectory ("odb-compiler")
add_subdirectory ("odb-interpreter")
