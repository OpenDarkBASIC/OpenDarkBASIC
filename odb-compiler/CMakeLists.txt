
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

find_package (FLEX REQUIRED )
find_package (BISON REQUIRED 3.7)
find_package (Gperf REQUIRED)

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
            ${BISON_CommandsParser_OUTPUTS}
        PROPERTY COMPILE_DEFINITIONS
            YYDEBUG)
endif ()

gperf_target (DarkBASICCommandToken
    "${PROJECT_SOURCE_DIR}/src/parsers/db/KeywordToken.gperf"
    "${PROJECT_BINARY_DIR}/src/parsers/db/KeywordToken.gperf.cpp")
set_source_files_properties (${Gperf_DarkBASICCommandToken_OUTPUTS}
    PROPERTIES COMPILE_FLAGS
        $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-field-initializers>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-missing-field-initializers>)
#XXX this is broken
add_gperf_bison_dependency (DarkBASICCommandToken DarkBASICParser)

###############################################################################
# Library definition
###############################################################################

add_library (odb-compiler ${ODBCOMPILER_LIB_TYPE}
    ${BISON_DarkBASICParser_OUTPUTS}
    ${FLEX_DarkBASICScanner_OUTPUTS}
    ${Gperf_DarkBASICCommandToken_OUTPUTS}
    ${BISON_CommandsParser_OUTPUTS}
    ${FLEX_CommandsScanner_OUTPUTS}
    "src/ast/ArrayRef.cpp"
    "src/ast/Assignment.cpp"
    "src/ast/BinaryOp.cpp"
    "src/ast/Block.cpp"
    "src/ast/Break.cpp"
    "src/ast/ConstDecl.cpp"
    "src/ast/Exporters_DOT.cpp"
    "src/ast/Exporters_JSON.cpp"
    "src/ast/Expression.cpp"
    "src/ast/ExpressionList.cpp"
    "src/ast/FuncCall.cpp"
    "src/ast/FuncDecl.cpp"
    "src/ast/Command.cpp"
    "src/ast/Literal.cpp"
    "src/ast/Loop.cpp"
    "src/ast/Node.cpp"
    "src/ast/SourceLocation.cpp"
    "src/ast/Statement.cpp"
    "src/ast/Symbol.cpp"
    "src/ast/VarDecl.cpp"
    "src/ast/VarRef.cpp"
    "src/ast/Visitor.cpp"
    "src/astpost/ErrorHighlighter.cpp"
    "src/astpost/Process.cpp"
    "src/astpost/ResolveArrayFuncAmbiguity.cpp"
    "src/commands/CommandLoader.cpp"
    "src/commands/ODBCommandLoader.cpp"
    "src/commands/DBPCommandLoader.cpp"
    "src/commands/Command.cpp"
    "src/commands/CommandIndex.cpp"
    "src/commands/CommandMatcher.cpp"
    "src/ir/Node.cpp"
    "src/parsers/db/Driver.cpp"
    "src/parsers/db/ErrorPrinter.cpp")
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
        "tests/src/test_db_assignment.cpp"
        # "tests/src/test_db_arrays.cpp"
        # "tests/src/test_db_conditional.cpp"
        "tests/src/test_db_constant.cpp"
        "tests/src/test_db_func_call.cpp"
        "tests/src/test_db_func_decl.cpp"
        # "tests/src/test_db_goto.cpp"
        "tests/src/test_db_command.cpp"
        "tests/src/test_db_loop_do.cpp"
        "tests/src/test_db_loop_for.cpp"
        "tests/src/test_db_loop_repeat.cpp"
        "tests/src/test_db_loop_while.cpp"
        "tests/src/test_db_op_add.cpp"
        # "tests/src/test_db_op_precedence.cpp"
        "tests/src/test_db_remarks.cpp"
        # "tests/src/test_db_select.cpp"
        # "tests/src/test_db_string_literal.cpp"
        # "tests/src/test_db_subroutine.cpp"
        # "tests/src/test_db_udt.cpp"
        "tests/src/test_db_var_decl.cpp"
        "tests/src/test_db_var_ref.cpp"
        # "tests/src/test_cmd_index.cpp"
        "tests/src/test_cmd_matcher.cpp"
        "tests/src/test_SourceLocation.cpp"
        "tests/src/ASTMatchers.cpp"
        "tests/src/ASTParentConsistenciesChecker.cpp"
        "tests/src/ParserTestHarness.cpp"
        "tests/src/main.cpp")
    target_link_libraries (odbc_tests
        PRIVATE
            gmock
            odb-compiler)
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
    "src/cli/Actions.cpp"
    "src/cli/AST.cpp"
    "src/cli/Banner.cpp"
    "src/cli/Commands.cpp"
    "src/cli/SDK.cpp"
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
