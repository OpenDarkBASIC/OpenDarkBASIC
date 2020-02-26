cmake_minimum_required (VERSION 3.13.0)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

project ("OpenDarkBASIC")

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

include (ConfigureMSVCRuntime)
configure_msvc_runtime()

add_subdirectory ("thirdparty/googletest")
add_subdirectory ("odb-compiler")
