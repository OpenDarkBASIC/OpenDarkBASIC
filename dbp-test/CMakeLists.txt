cmake_minimum_required (VERSION 3.21)

project (dbp-test
    LANGUAGES C)

add_executable (dbp-test
    "src/runtime.c"
    "src/main.c")
target_include_directories (dbp-test
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
set_target_properties (dbp-test
    PROPERTIES
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
