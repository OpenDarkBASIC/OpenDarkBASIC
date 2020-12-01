project ("odb-util"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)

###############################################################################
# Configuration
###############################################################################

set (ODBUTIL_LIB_TYPE "SHARED" CACHE STRING "Build either as SHARED or STATIC library")

test_visibility_macros (
    ODBUTIL_API_EXPORT
    ODBUTIL_API_IMPORT
    ODBUTIL_API_LOCAL)

configure_file ("templates/config.hpp.in" "include/odb-util/config.hpp")

###############################################################################
# Library definition
###############################################################################

add_library (odb-util ${ODBUTIL_LIB_TYPE}
    "src/Log.cpp"
    "src/Str.cpp"
    $<$<PLATFORM_ID:Linux>:src/platform/linux/FileSystem_linux.cpp>
    $<$<PLATFORM_ID:Windows>:src/platform/win32/FileSystem_win32.cpp>)
target_include_directories (odb-util
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (odb-util
    PRIVATE
        ODBUTIL_BUILDING)
target_compile_options(odb-util
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
target_compile_features (odb-util
    PUBLIC
        cxx_std_17)
