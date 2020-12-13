
project ("odb-compiler"
    LANGUAGES C CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)
include (CheckIncludeFile)

###############################################################################
# Configuration
###############################################################################

set (ODBCOMPILER_LIB_TYPE "SHARED" CACHE STRING "Build as either SHARED or STATIC library")
option (ODBCOMPILER_TESTS "Build unit tests" ON)
option (ODBCOMPILER_DOT_EXPORT "Enable functions for dumping AST to DOT format" ON)
option (ODBCOMPILER_VERBOSE_BISON "Compile with YYDEBUG and enable verbose bison output" OFF)
option (ODBCOMPILER_VERBOSE_FLEX "Have the scanner output each token" OFF)
option (ODBCOMPILER_BISON_COUNTER_EXAMPLES "Tell bison to provide counter examples when grammar contains conflicts" OFF)

test_visibility_macros (
    ODBCOMPILER_API_EXPORT
    ODBCOMPILER_API_IMPORT
    ODBCOMPILER_API_LOCAL)

check_include_file ("unistd.h" HAVE_UNISTD_H)

if (WIN32)
    set (ODB_PLATFORM "WIN32")
elseif (APPLE)
    set (ODB_PLATFORM "MACOS")
elseif (UNIX)
    set (ODB_PLATFORM "LINUX")
endif ()

configure_file ("templates/config.hpp.in" "include/odb-compiler/config.hpp")

###############################################################################
# Parsers
###############################################################################

find_package (FLEX REQUIRED)
find_package (BISON REQUIRED 3.2)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parsers/db")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-compiler/parsers/db")

set (BISON_VERBOSE_OPTION "")
if (${ODBCOMPILER_VERBOSE_BISON})
    set (BISON_VERBOSE_OPTION "VERBOSE")
endif ()

if (ODBCOMPILER_BISON_COUNTER_EXAMPLES)
    set (BISON_COMPILE_FLAGS -Wcounterexamples)
endif ()
bison_target (DarkBASICParser
    "${PROJECT_SOURCE_DIR}/src/parsers/db/Parser.y"
    "${PROJECT_BINARY_DIR}/src/parsers/db/Parser.y.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odb-compiler/parsers/db/Parser.y.h" ${BISON_VERBOSE_OPTION}
    COMPILE_FLAGS ${BISON_COMPILE_FLAGS})
flex_target (DarkBASICScanner
    "${PROJECT_SOURCE_DIR}/src/parsers/db/Scanner.lex"
    "${PROJECT_BINARY_DIR}/src/parsers/db/Scanner.lex.cpp")
add_flex_bison_dependency (DarkBASICScanner DarkBASICParser)

if (ODBCOMPILER_VERBOSE_BISON)
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

add_library (odb-compiler ${ODBCOMPILER_LIB_TYPE}
    ${BISON_DarkBASICParser_OUTPUTS}
    ${FLEX_DarkBASICScanner_OUTPUTS}
    ${BISON_KeywordsParser_OUTPUTS}
    ${FLEX_KeywordsScanner_OUTPUTS}
    "src/ast/ErrorHighlighter.cpp"
    "src/ast/Node.cpp"
    "src/ast/OldNode.cpp"
    "src/ast/Node_dumpToDOT.cpp"
    "src/ast/Node_dumpToJSON.cpp"
    "src/ast/Process.cpp"
    "src/ast/ResolveArrayFuncAmbiguity.cpp"
    "src/ast/SourceLocation.cpp"
    "src/ast/Visitor.cpp"
    "src/keywords/KeywordLoader.cpp"
    "src/keywords/ODBKeywordLoader.cpp"
    "src/keywords/DBPKeywordLoader.cpp"
    "src/keywords/Keyword.cpp"
    "src/keywords/KeywordIndex.cpp"
    "src/keywords/KeywordMatcher.cpp"
    "src/parsers/db/Driver.cpp")
target_include_directories (odb-compiler
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        $<BUILD_INTERFACE:$<$<AND:$<PLATFORM_ID:Windows>,$<NOT:$<BOOL:${HAVE_UNISTD_H}>>>:${PROJECT_SOURCE_DIR}/include/win32_unistd>>)
target_compile_definitions (odb-compiler
    PRIVATE
        ODBCOMPILER_BUILDING)
target_compile_options (odb-compiler
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
target_compile_features (odb-compiler
    PUBLIC cxx_std_17)
target_link_libraries (odb-compiler
    PUBLIC
        odb-sdk)
set_target_properties (odb-compiler
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_ARCHIVE_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_LIBRARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})

if (${ODBCOMPILER_TESTS})
    add_executable (odbc_tests
        # "tests/src/test_db_arrays.cpp"
        # "tests/src/test_db_conditional.cpp"
        "tests/src/test_db_constant.cpp"
        "tests/src/test_db_func_call.cpp"
        # "tests/src/test_db_func_decl.cpp"
        # "tests/src/test_db_goto.cpp"
        "tests/src/test_db_keyword.cpp"
        # "tests/src/test_db_loop_do.cpp"
        # "tests/src/test_db_loop_for.cpp"
        # "tests/src/test_db_loop_repeat.cpp"
        # "tests/src/test_db_loop_while.cpp"
        # "tests/src/test_db_op_add.cpp"
        # "tests/src/test_db_op_precedence.cpp"
        "tests/src/test_db_remarks.cpp"
        # "tests/src/test_db_select.cpp"
        # "tests/src/test_db_string_literal.cpp"
        # "tests/src/test_db_subroutine.cpp"
        # "tests/src/test_db_udt.cpp"
        "tests/src/test_db_var_decl.cpp"
        # "tests/src/test_db_var_ref.cpp"
        # "tests/src/test_kw_index.cpp"
        "tests/src/test_kw_matcher.cpp"
        "tests/src/test_SourceLocation.cpp"
        "tests/src/ASTMatchers.cpp"
        "tests/src/main.cpp")
    target_link_libraries (odbc_tests
        PRIVATE
            gmock
            odb-compiler
            $<$<NOT:$<PLATFORM_ID:Windows>>:stdc++fs>)
    target_include_directories (odbc_tests
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/include>)
    target_compile_features (odbc_tests
        PUBLIC
            cxx_std_17)
    set_target_properties (odbc_tests
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})
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
        odb-compiler)
set_target_properties (odbc
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odb-compiler
    EXPORT OpenDarkBASICConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install (
    DIRECTORY "include/odb-compiler"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-compiler"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (
    EXPORT OpenDarkBASICConfig
    DESTINATION "share/odb/cmake")

install (
    TARGETS odbc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
