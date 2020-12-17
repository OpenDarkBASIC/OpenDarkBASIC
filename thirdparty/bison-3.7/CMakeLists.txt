cmake_minimum_required (VERSION 3.3)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

project ("GNU Bison")

add_subdirectory ("bison-util")
add_subdirectory ("bison")
