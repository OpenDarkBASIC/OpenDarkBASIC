project (dbp-runtime
    LANGUAGES C)

configure_file ("templates/config.h.in" "include/dbp-runtime/config.h")

add_library (dbp-runtime SHARED
    "src/dbprt.c")
target_include_directories (dbp-runtime
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
include (ODBTargetProperties)
odb_target_properties (dbp-runtime
    MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
    ARCHIVE_OUTPUT_DIRECTORY "${DBP_BUILD_SDKDIR}/runtime"
    LIBRARY_OUTPUT_DIRECTORY "${DBP_BUILD_SDKDIR}/runtime"
    RUNTIME_OUTPUT_DIRECTORY "${DBP_BUILD_SDKDIR}/runtime")
