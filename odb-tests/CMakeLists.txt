project (odb-tests
    LANGUAGES CXX)

add_executable (odb-tests
    "src/env_odbutil_init.cpp"
    "src/main.cpp")
target_link_libraries (odb-tests
    PRIVATE
        gmock)
target_compile_options (odb-tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)
target_compile_definitions (odb-tests
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_HAS_EXCEPTIONS=0>)
include (ODBTargetProperties)
odb_target_properties (odb-tests
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR}
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})
