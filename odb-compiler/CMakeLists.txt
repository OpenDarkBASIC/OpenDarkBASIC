project ("odbc"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)
include (ConfigureMSVCRuntime)

###############################################################################
# Configuration
###############################################################################

set (ODBC_LIB_TYPE "SHARED" CACHE STRING "Build as either SHARED or STATIC library")
option (ODBC_TESTS "Build unit tests" ON)
option (ODBC_DOT_EXPORT "Enable functions for dumping AST to DOT format" ON)

test_visibility_macros (
    ODBC_API_IMPORT
    ODBC_API_EXPORT
    ODBC_API_LOCAL)

configure_file ("templates/config.hpp.in" "include/odbc/config.hpp")

###############################################################################
# Parser
###############################################################################

find_package (FLEX REQUIRED)
find_package (BISON REQUIRED)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc")

bison_target (ODBCParser
    "${PROJECT_SOURCE_DIR}/src/Parser.y"
    "${PROJECT_BINARY_DIR}/src/Parser.y.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odbc/Parser.y.h")
flex_target (ODBCScanner
    "${PROJECT_SOURCE_DIR}/src/Scanner.lex"
    "${PROJECT_BINARY_DIR}/src/Scanner.lex.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odbc/Scanner.lex.h")
add_flex_bison_dependency (ODBCScanner ODBCParser)

###############################################################################
# Library definition
###############################################################################

add_library (odbc ${ODBC_LIB_TYPE}
    ${BISON_ODBCParser_OUTPUTS}
    ${FLEX_ODBCScanner_OUTPUTS}
    "src/ASTNode.cpp"
    "src/Driver.cpp")
target_include_directories (odbc
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (odbc
    PRIVATE
        ODBC_BUILDING)
target_compile_options (odbc
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-unused-parameter>)

if (${ODBC_TESTS})
    add_executable (odbc_tests
        "tests/src/test_ast_conditional.cpp"
        "tests/src/test_ast_constant.cpp"
        "tests/src/test_ast_function_call.cpp"
        "tests/src/test_ast_loop_do.cpp"
        "tests/src/test_ast_loop_for.cpp"
        "tests/src/test_ast_loop_repeat.cpp"
        "tests/src/test_ast_loop_while.cpp"
        "tests/src/test_ast_op_add.cpp"
        "tests/src/test_ast_remarks.cpp")
    target_link_libraries (odbc_tests
        PRIVATE
            odbc
            gmock
            gmock_main
            stdc++fs)
    target_include_directories (odbc_tests
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/include>)
    set_target_properties (odbc_tests PROPERTIES CXX_STANDARD 17)
endif ()

###############################################################################
# Installation
###############################################################################

if (${ODBC_LIB_TYPE} MATCHES "SHARED")
    install (
        TARGETS odbc
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (${INSTALL_DEV})
    install (
        TARGETS odbc
        EXPORT ODBCConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install (
        DIRECTORY "include/odbc"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install (
        DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install (
        EXPORT ODBCConfig
        DESTINATION "share/odbc/cmake")
endif ()

export (
    TARGETS odbc
    NAMESPACE ODBC
    FILE ODBCConfig.cmake)
export (
    PACKAGE ODBC)
