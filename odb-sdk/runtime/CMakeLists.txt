project ("odb-runtime"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)

###############################################################################
# Configuration
###############################################################################

set (ODBRUNTIME_LIB_TYPE "SHARED" CACHE STRING "Build either as SHARED or STATIC library")

test_visibility_macros (
    ODBRUNTIME_API_EXPORT
    ODBRUNTIME_API_IMPORT
    ODBRUNTIME_API_LOCAL)

configure_file ("templates/config.hpp.in" "include/odb-sdk/runtime/config.hpp")

###############################################################################
# Library definition
###############################################################################

add_library (odb-runtime ${ODBRUNTIME_LIB_TYPE}
    "src/Plugin.cpp"
    $<$<PLATFORM_ID:Linux>:src/platform/linux/Plugin_linux.cpp>)
target_include_directories (odb-runtime
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (odb-runtime
    PRIVATE
        ODBSDK_BUILDING)
target_compile_options (odb-runtime
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
target_compile_features (odb-runtime
    PUBLIC cxx_std_17)

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (odb-runtime PRIVATE DL::DL)
endif ()
