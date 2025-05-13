project (odb-tests
    LANGUAGES CXX)

if (WIN32)
    set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    include (FetchContent)
    FetchContent_Declare (
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7)
    FetchContent_MakeAvailable (googletest)
    include (ODBTargetProperties)
    odb_target_properties (gmock
        PROPERTIES
            MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
    add_library (GTest::gmock ALIAS gmock)
else ()
    find_package (GTest REQUIRED)
endif ()

add_executable (odb-tests
    "src/env_odbutil_init.cpp"
    "src/main.cpp")
target_link_libraries (odb-tests
    PRIVATE
        GTest::gmock)
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

install (
    TARGETS odb-tests
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})
