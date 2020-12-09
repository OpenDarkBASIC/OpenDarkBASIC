project ("odb-sdk"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)

###############################################################################
# Configuration
###############################################################################

set (ODBSDK_LIB_TYPE "SHARED" CACHE STRING "Build either as SHARED or STATIC library")

test_visibility_macros (
    ODBSDK_API_EXPORT
    ODBSDK_API_IMPORT
    ODBSDK_API_LOCAL)

configure_file ("templates/config.hpp.in" "include/odb-sdk/config.hpp")

###############################################################################
# Library definition
###############################################################################

add_library (odb-sdk ${ODBSDK_LIB_TYPE}
    "src/DynamicLibrary.cpp"
    "src/FileSystem.cpp"
    "src/Log.cpp"
    "src/RefCounted.cpp"
    "src/Str.cpp")
target_include_directories (odb-sdk
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (odb-sdk
    PRIVATE
        ODBSDK_BUILDING)
target_compile_options (odb-sdk
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
target_compile_features (odb-sdk
    PUBLIC cxx_std_17)

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (odb-sdk PRIVATE DL::DL)
endif ()

add_subdirectory ("plugins")
