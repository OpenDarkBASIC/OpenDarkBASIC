project ("odbc"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)
include (CheckIncludeFile)

find_package(LLVM REQUIRED CONFIG)

###############################################################################
# Configuration
###############################################################################

set (ODBC_LIB_TYPE "SHARED" CACHE STRING "Build as either SHARED or STATIC library")
option (ODBC_TESTS "Build unit tests" ON)
option (ODBC_DOT_EXPORT "Enable functions for dumping AST to DOT format" ON)
option (ODBC_VERBOSE_BISON "Compile with YYDEBUG and enable verbose bison output" OFF)
option (ODBC_VERBOSE_FLEX "Have the scanner output each token" OFF)

test_visibility_macros (
    ODBC_API_EXPORT
    ODBC_API_IMPORT
    ODBC_API_LOCAL)

check_include_file ("unistd.h" HAVE_UNISTD_H)

configure_file ("templates/config.hpp.in" "include/odbc/config.hpp")

###############################################################################
# Parsers
###############################################################################

find_package (FLEX REQUIRED)
find_package (BISON REQUIRED 3.2)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parsers/db")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parsers/keywords")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc/parsers/db")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc/parsers/keywords")

set (BISON_VERBOSE_OPTION "")
if (${ODBC_VERBOSE_BISON})
    set (BISON_VERBOSE_OPTION "VERBOSE")
endif ()

bison_target (DarkBASICParser
    "${PROJECT_SOURCE_DIR}/src/parsers/db/Parser.y"
    "${PROJECT_BINARY_DIR}/src/parsers/db/Parser.y.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odbc/parsers/db/Parser.y.h" ${BISON_VERBOSE_OPTION})
flex_target (DarkBASICScanner
    "${PROJECT_SOURCE_DIR}/src/parsers/db/Scanner.lex"
    "${PROJECT_BINARY_DIR}/src/parsers/db/Scanner.lex.cpp")
add_flex_bison_dependency (DarkBASICScanner DarkBASICParser)

bison_target (KeywordsParser
    "${PROJECT_SOURCE_DIR}/src/parsers/keywords/Parser.y"
    "${PROJECT_BINARY_DIR}/src/parsers/keywords/Parser.y.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odbc/parsers/keywords/Parser.y.h" ${BISON_VERBOSE_OPTION})
flex_target (KeywordsScanner
    "${PROJECT_SOURCE_DIR}/src/parsers/keywords/Scanner.lex"
    "${PROJECT_BINARY_DIR}/src/parsers/keywords/Scanner.lex.cpp")
add_flex_bison_dependency (KeywordsScanner KeywordsParser)

if (ODBC_VERBOSE_BISON)
    set_property (
        SOURCE
            ${BISON_DarkBASICParser_OUTPUTS}
            ${BISON_KeywordsParser_OUTPUTS}
        PROPERTY COMPILE_DEFINITIONS
            YYDEBUG)
endif ()

###############################################################################
# Library definition
###############################################################################

add_library (odbc ${ODBC_LIB_TYPE}
    ${BISON_DarkBASICParser_OUTPUTS}
    ${FLEX_DarkBASICScanner_OUTPUTS}
    ${BISON_KeywordsParser_OUTPUTS}
    ${FLEX_KeywordsScanner_OUTPUTS}
    "src/ast/Node.cpp"
    "src/ast/Node2.cpp"
    "src/ast/Node_dumpToDOT.cpp"
    "src/ast/Node_dumpToJSON.cpp"
    "src/ast/Node_dumpToLLVM.cpp"
    "src/parsers/db/Driver.cpp"
    "src/parsers/keywords/Driver.cpp"
    "src/parsers/keywords/KeywordDB.cpp"
    "src/parsers/keywords/KeywordDLL.cpp"
    "src/parsers/keywords/KeywordMatcher.cpp"
    "src/util/Log.cpp"
    "src/util/Str.cpp"
    $<$<PLATFORM_ID:Linux>:src/util/platform/linux/Plugin_linux.cpp>
    $<$<PLATFORM_ID:Linux>:src/util/platform/linux/FileSystem_linux.cpp>
    $<$<PLATFORM_ID:Darwin>:src/util/platform/darwin/FileSystem_darwin.cpp>
    $<$<PLATFORM_ID:Windows>:src/util/platform/win32/FileSystem_win32.cpp>)
target_include_directories (odbc
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:$<$<AND:$<PLATFORM_ID:Windows>,$<NOT:$<BOOL:${HAVE_UNISTD_H}>>>:${PROJECT_SOURCE_DIR}/include/win32_unistd>>)
target_compile_definitions (odbc
    PRIVATE
        ODBC_BUILDING)
target_compile_options (odbc
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
target_compile_features (odbc PUBLIC cxx_std_17)
target_include_directories(odbc PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(odbc PUBLIC ${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs Core)
target_link_libraries(odbc PUBLIC ${llvm_libs})

if (${ODBC_TESTS})
    add_executable (odbc_tests
        "tests/src/test_db_arrays.cpp"
        "tests/src/test_db_conditional.cpp"
        "tests/src/test_db_constant.cpp"
        "tests/src/test_db_function_call.cpp"
        "tests/src/test_db_function_decl.cpp"
        "tests/src/test_db_goto.cpp"
        "tests/src/test_db_keyword.cpp"
        "tests/src/test_db_loop_do.cpp"
        "tests/src/test_db_loop_for.cpp"
        "tests/src/test_db_loop_repeat.cpp"
        "tests/src/test_db_loop_while.cpp"
        "tests/src/test_db_op_add.cpp"
        "tests/src/test_db_op_precedence.cpp"
        "tests/src/test_db_remarks.cpp"
        "tests/src/test_db_select.cpp"
        "tests/src/test_db_string_literal.cpp"
        "tests/src/test_db_subroutine.cpp"
        "tests/src/test_db_udt.cpp"
        "tests/src/test_db_variables.cpp"
        "tests/src/test_kw_index.cpp"
        "tests/src/test_kw_matcher.cpp"
        "tests/src/main.cpp")
    target_link_libraries (odbc_tests
        PRIVATE
            odbc
            gmock
            $<$<NOT:$<PLATFORM_ID:Windows>>:stdc++fs>)
    target_include_directories (odbc_tests
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/include>)
    set_target_properties (odbc_tests PROPERTIES CXX_STANDARD 17)
endif ()

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (odbc PRIVATE DL::DL)
endif ()

###############################################################################
# CLI Executable
###############################################################################

add_executable (odbc_cli
    "src/cli/Args.cpp"
    "src/cli/main.cpp")
target_include_directories (odbc_cli
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries (odbc_cli
    PRIVATE
        odbc)
set_target_properties (odbc_cli
    PROPERTIES
        OUTPUT_NAME "odbc")

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
