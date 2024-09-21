cmake_minimum_required (VERSION 3.21)

project (dbp-stdout
    LANGUAGES C
    VERSION 0.0.1)

add_library (dbp-stdout SHARED
    "src/stdout.c"
    "src/stdout.rc")

include (ODBTargetProperties)
odb_target_properties (dbp-stdout
    PROPERTIES
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        RUNTIME_OUTPUT_DIRECTORY "${DBP_BUILD_SDKDIR}/plugins-user")
