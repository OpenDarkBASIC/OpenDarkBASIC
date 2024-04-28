cmake_minimum_required (VERSION 3.24)

project (dbp-runtime
    LANGUAGES C)

add_library (dbp-runtime STATIC
    "include/dbp-runtime/globstruct.h"
    "src/runtime.c")
target_include_directories (dbp-runtime
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

add_executable (dbp-runtime-exec
    "src/main.c")
target_link_libraries (dbp-runtime-exec PRIVATE dbp-runtime)
