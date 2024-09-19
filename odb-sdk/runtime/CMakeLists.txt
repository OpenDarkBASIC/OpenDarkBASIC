project (odb-runtime
    LANGUAGES C)

configure_file ("templates/config.h.in" "include/odb-runtime/config.h")

add_library (odb-runtime SHARED
    "src/odbrt.c")
target_include_directories (odb-runtime
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries (odb-runtime
    PRIVATE
        odb-util)
include (ODBTargetProperties)
odb_target_properties (odb-runtime
    MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
    ARCHIVE_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/runtime"
    LIBRARY_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/runtime"
    RUNTIME_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/runtime")
