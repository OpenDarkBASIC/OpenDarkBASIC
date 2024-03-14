project ("odb-sdk"
    LANGUAGES CXX
    VERSION ${ODB_VERSION})

###############################################################################
# Configuration
###############################################################################

include (CMakeDependentOption)
set (ODBSDK_LIB_TYPE "SHARED" CACHE STRING "Build either as SHARED or STATIC library")
cmake_dependent_option (ODBSDK_TESTS "Build unit tests for the OpenDarkBASIC SDK" ON "${ODB_TESTS}" OFF)

include (TestVisibilityMacros)
test_visibility_macros (
    ODBSDK_API_EXPORT
    ODBSDK_API_IMPORT
    ODBSDK_API_LOCAL)

configure_file ("templates/config.hpp.in" "include/odb-sdk/config.hpp")

###############################################################################
# Library definition
###############################################################################

add_library (odb-sdk ${ODBSDK_LIB_TYPE}
    "include/odb-sdk/DynamicLibrary.hpp"

    "$<$<PLATFORM_ID:Windows>:src/DynamicLibrary_win32.cpp>"
	"$<$<PLATFORM_ID:Linux>:src/DynamicLibrary_linux.cpp>"
    #"src/FileSystem.cpp"
    "src/Log.cpp"
    "src/RefCounted.cpp"
    "src/Str.cpp")
target_include_directories (odb-sdk
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include/odb-sdk>")
target_compile_definitions (odb-sdk
    PRIVATE
        ODBSDK_BUILDING
        _CRT_SECURE_NO_WARNINGS)
target_compile_options (odb-sdk
    PRIVATE
        "$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>"
        "$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>"
        "$<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>")
include (ODBTargetProperties)
odb_target_properties (odb-sdk
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (odb-sdk PRIVATE DL::DL)
endif ()

if (ODBSDK_TESTS)
    add_executable (test_odb-sdk
        "tests/test_DynamicLibrary.cpp")
    target_link_libraries (test_odb-sdk PUBLIC
        odb-sdk
        gmock
        gmock_main)
    odb_target_properties (test_odb-sdk PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR}
        VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR})
endif ()

add_subdirectory ("plugins")
add_subdirectory ("runtime")
add_subdirectory ("runtime-dbp")

install (
    TARGETS odb-sdk
    EXPORT OpenDarkBASICConfig
    ARCHIVE DESTINATION ${ODB_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${ODB_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})
install (
    DIRECTORY "include/odb-sdk"
    DESTINATION ${ODB_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-sdk"
    DESTINATION ${ODB_INSTALL_INCLUDEDIR})
install (
    EXPORT OpenDarkBASICConfig
    DESTINATION "share/odb/cmake")
