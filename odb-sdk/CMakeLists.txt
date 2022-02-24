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
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_ODBSDK_INCLUDEDIR}>)
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
set_target_properties (odb-sdk
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_ARCHIVE_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_LIBRARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (odb-sdk PRIVATE DL::DL)
endif ()

add_subdirectory ("plugins")
add_subdirectory ("runtime")
add_subdirectory ("runtime-dbp")

install (
    TARGETS odb-sdk
    EXPORT OpenDarkBASICConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install (
    DIRECTORY "include/odb-sdk"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-sdk"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (
    EXPORT OpenDarkBASICConfig
    DESTINATION "share/odb/cmake")
