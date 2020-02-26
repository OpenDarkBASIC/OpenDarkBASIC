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
# Parsers
###############################################################################

find_package (FLEX REQUIRED)
find_package (BISON REQUIRED)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parsers/db")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parsers/keywords")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc/parsers/db")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc/parsers/keywords")

bison_target (DarkBASICParser
    "${PROJECT_SOURCE_DIR}/src/parsers/db/Parser.y"
    "${PROJECT_BINARY_DIR}/src/parsers/db/Parser.y.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odbc/parsers/db/Parser.y.h")
flex_target (DarkBASICScanner
    "${PROJECT_SOURCE_DIR}/src/parsers/db/Scanner.lex"
    "${PROJECT_BINARY_DIR}/src/parsers/db/Scanner.lex.cpp")
add_flex_bison_dependency (DarkBASICScanner DarkBASICParser)

bison_target (KeywordsParser
    "${PROJECT_SOURCE_DIR}/src/parsers/keywords/Parser.y"
    "${PROJECT_BINARY_DIR}/src/parsers/keywords/Parser.y.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odbc/parsers/keywords/Parser.y.h")
flex_target (KeywordsScanner
    "${PROJECT_SOURCE_DIR}/src/parsers/keywords/Scanner.lex"
    "${PROJECT_BINARY_DIR}/src/parsers/keywords/Scanner.lex.cpp")
add_flex_bison_dependency (KeywordsScanner KeywordsParser)

###############################################################################
# Library definition
###############################################################################

add_library (odbclib ${ODBC_LIB_TYPE}
    ${BISON_DarkBASICParser_OUTPUTS}
    ${FLEX_DarkBASICScanner_OUTPUTS}
    ${BISON_KeywordsParser_OUTPUTS}
    ${FLEX_KeywordsScanner_OUTPUTS}
    "src/ast/Node.cpp"
    "src/logging/Log.cpp"
    "src/parsers/db/Driver.cpp"
    "src/parsers/keywords/Driver.cpp"
    "src/parsers/keywords/KeywordDB.cpp"
    "src/parsers/keywords/KeywordMatcher.cpp")
target_include_directories (odbclib
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (odbclib
    PRIVATE
        ODBC_BUILDING
        YYDEBUG)
target_compile_options (odbclib
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-unused-parameter>)
set_target_properties (odbclib PROPERTIES CXX_STANDARD 17)

if (${ODBC_TESTS})
    add_executable (odbc_tests
        "tests/src/test_db_keyword.cpp"
        "tests/src/test_db_conditional.cpp"
        "tests/src/test_db_constant.cpp"
        "tests/src/test_db_dim.cpp"
        "tests/src/test_db_function_call.cpp"
        "tests/src/test_db_function_decl.cpp"
        "tests/src/test_db_loop_do.cpp"
        "tests/src/test_db_loop_for.cpp"
        "tests/src/test_db_loop_repeat.cpp"
        "tests/src/test_db_loop_while.cpp"
        "tests/src/test_db_op_add.cpp"
        "tests/src/test_db_remarks.cpp"
        "tests/src/test_db_sub.cpp"
        "tests/src/test_db_udt.cpp"
        "tests/src/test_db_symbol_to_keyword_hack.cpp"
        "tests/src/test_kw_index.cpp"
        "tests/src/test_kw_matcher.cpp"
        "tests/src/main.cpp")
    target_link_libraries (odbc_tests
        PRIVATE
            odbclib
            gmock
            stdc++fs)
    target_include_directories (odbc_tests
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/include>)
    set_target_properties (odbc_tests PROPERTIES CXX_STANDARD 17)
endif ()

###############################################################################
# CLI Executable
###############################################################################

add_executable (odbc
    "src/cli/Args.cpp"
    "src/cli/main.cpp")
target_include_directories (odbc
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries (odbc
    PRIVATE
        odbclib)

###############################################################################
# Installation
###############################################################################

if (${ODBC_LIB_TYPE} MATCHES "SHARED")
    install (
        TARGETS odbclib
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (${INSTALL_DEV})
    install (
        TARGETS odbclib
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
    TARGETS odbclib
    NAMESPACE ODBC
    FILE ODBCConfig.cmake)
export (
    PACKAGE ODBC)
