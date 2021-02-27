project ("odb-compiler"
    LANGUAGES C CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)
include (CheckIncludeFile)

###############################################################################
# Configuration
###############################################################################

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set (DEBUG_FEATURE ON)
else ()
    set (RELEASE_FEATURE ON)
endif ()

set (ODBCOMPILER_LIB_TYPE "SHARED" CACHE STRING "Build as either SHARED or STATIC library")
option (ODBCOMPILER_BISON_COUNTER_EXAMPLES "Tell bison to provide counter examples when grammar contains conflicts" OFF)
option (ODBCOMPILER_UPDATE_BUILDINFO "If true the updated build number is written to a header, which causes the library to recompile BuildInfo.c. This can get annoying for developers, so disable this if you are annoyed." ${RELEASE_FEATURE})
option (ODBCOMPILER_DOT_EXPORT "Enable functions for dumping AST to DOT format" ON)
option (ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS "Link with a shared library build of LLVM" OFF)
option (ODBCOMPILER_PIC "Enable building with -fPIC (position independent code)" ON)
option (ODBCOMPILER_VERBOSE_BISON "Compile with YYDEBUG and enable verbose bison output" OFF)
option (ODBCOMPILER_VERBOSE_FLEX "Have the scanner output each token" OFF)
option (ODBCOMPILER_TESTS "Build unit tests" ON)

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
# Build info
###############################################################################

set (ODBCOMPILER_HOST_COMPUTER ${CMAKE_HOST_SYSTEM})
set (ODBCOMPILER_COMPILER ${CMAKE_C_COMPILER_ID})
find_program (UNAME_PROGRAM uname)
if (UNAME_PROGRAM)
    execute_process (
        COMMAND ${UNAME_PROGRAM} -a
        OUTPUT_VARIABLE ODBCOMPILER_HOST_COMPUTER)
    string (REPLACE "\n" "" ODBCOMPILER_HOST_COMPUTER ${ODBCOMPILER_HOST_COMPUTER})
else ()
    set (ODBCOMPILER_HOST_COMPUTER ${CMAKE_HOST_SYSTEM_NAME})
endif ()

# Required for build info
find_package (Git)
if (GIT_FOUND)
    execute_process (COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE RESULT OUTPUT_VARIABLE ODBCOMPILER_COMMIT_TAG)
    if (RESULT)
        set (ODBCOMPILER_COMMIT_TAG "(unknown tag)")
    else ()
        string (REPLACE "\n" "" ODBCOMPILER_COMMIT_TAG ${ODBCOMPILER_COMMIT_TAG})
    endif ()
    execute_process (COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD RESULT_VARIABLE RESULT OUTPUT_VARIABLE ODBCOMPILER_COMMIT_HASH)
    if (RESULT)
        set (ODBCOMPILER_COMMIT_HASH "(commit hash unknown)")
    else ()
        string (REPLACE "\n" "" ODBCOMPILER_COMMIT_HASH ${ODBCOMPILER_COMMIT_HASH})
    endif ()
    set (ODBCOMPILER_COMMIT_INFO "${ODBCOMPILER_COMMIT_TAG} (${ODBCOMPILER_COMMIT_HASH})")
else ()
    set (ODBCOMPILER_COMMIT_INFO "unknown (git not found!)")
    message (WARNING "Git not found. Build will not contain git revision info.")
endif ()

# Update build info every time the project is compiled
set (BULIDINFOGENERATED_HEADER "${PROJECT_BINARY_DIR}/include/odb-compiler/BuildInfoGenerated.hpp")
add_custom_target (odbcompiler_build_info_generated
    COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_FILE=${BULIDINFOGENERATED_HEADER}
            -DODBCOMPILER_UPDATE_BUILDINFO=${ODBCOMPILER_UPDATE_BUILDINFO}
            -P "${PROJECT_SOURCE_DIR}/cmake/UpdateBuildInfoHeader.cmake"
    COMMENT "Updating build information"
    VERBATIM)
add_custom_command (
    OUTPUT ${BULIDINFOGENERATED_HEADER}
    COMMAND ${CMAKE_COMMAND}
            -P "${PROJECT_SOURCE_DIR}/cmake/nop.cmake"
    DEPENDS "cmake/UpdateBuildInfoHeader.cmake"
            "templates/BuildInfo.cpp.in"
            odbcompiler_build_info_generated
    VERBATIM)

configure_file ("templates/BuildInfo.cpp.in" "src/BuildInfo.cpp")

###############################################################################
# Parsers
###############################################################################

find_package (FLEX REQUIRED 2.6)
find_package (BISON REQUIRED 3.7)

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
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odb-compiler/parsers/db/Parser.y.hpp" ${BISON_VERBOSE_OPTION}
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

gperf_target (DarkBASICKeywordToken
    "${PROJECT_SOURCE_DIR}/src/parsers/db/KeywordToken.gperf"
    "${PROJECT_BINARY_DIR}/src/parsers/db/KeywordToken.gperf.cpp")
set_source_files_properties (${Gperf_DarkBASICKeywordToken_OUTPUTS}
    PROPERTIES COMPILE_FLAGS
        $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-field-initializers>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-missing-field-initializers>)
#XXX this is broken
add_gperf_bison_dependency (DarkBASICKeywordToken DarkBASICParser)

###############################################################################
# Library definition
###############################################################################

add_library (odb-compiler ${ODBCOMPILER_LIB_TYPE}
    "${BULIDINFOGENERATED_HEADER}"
    "${PROJECT_BINARY_DIR}/src/BuildInfo.cpp"
    "${BISON_DarkBASICParser_OUTPUTS}"
    "${FLEX_DarkBASICScanner_OUTPUTS}"
    "${Gperf_DarkBASICKeywordToken_OUTPUTS}"
    "${BISON_CommandsParser_OUTPUTS}"
    "${FLEX_CommandsScanner_OUTPUTS}"
    "src/ast/Annotation.cpp"
    "src/ast/AnnotatedSymbol.cpp"
    "src/ast/ArgList.cpp"
    "src/ast/ArrayDecl.cpp"
    "src/ast/ArrayRef.cpp"
    "src/ast/Assignment.cpp"
    "src/ast/BinaryOp.cpp"
    "src/ast/Block.cpp"
    "src/ast/CommandExpr.cpp"
    "src/ast/CommandStmnt.cpp"
    "src/ast/Conditional.cpp"
    "src/ast/ConstDecl.cpp"
    "src/ast/Exit.cpp"
    "src/ast/Exporters_DOT.cpp"
    "src/ast/Exporters_JSON.cpp"
    "src/ast/Expression.cpp"
    "src/ast/FuncCall.cpp"
    "src/ast/FuncDecl.cpp"
    "src/ast/Goto.cpp"
    "src/ast/InitializerList.cpp"
    "src/ast/Label.cpp"
    "src/ast/Literal.cpp"
    "src/ast/Loop.cpp"
    "src/ast/LValue.cpp"
    "src/ast/Node.cpp"
    "src/ast/Operator.cpp"
    "src/ast/Scope.cpp"
    "src/ast/ScopedAnnotatedSymbol.cpp"
    "src/ast/SelectCase.cpp"
    "src/ast/SourceLocation.cpp"
    "src/ast/Statement.cpp"
    "src/ast/Subroutine.cpp"
    "src/ast/Symbol.cpp"
    "src/ast/UDTDecl.cpp"
    "src/ast/UDTField.cpp"
    "src/ast/UDTRef.cpp"
    "src/ast/UnaryOp.cpp"
    "src/ast/VarDecl.cpp"
    "src/ast/VarRef.cpp"
    "src/ast/Visitor.cpp"
    "src/astpost/EliminateBitwiseNotRHS.cpp"
    "src/astpost/EnforceSingleDefaultCase.cpp"
    "src/astpost/Process.cpp"
    "src/astpost/ValidateUDTFieldNames.cpp"
    "src/commands/CommandLoader.cpp"
    "src/commands/ODBCommandLoader.cpp"
    "src/commands/DBPCommandLoader.cpp"
    "src/commands/Command.cpp"
    "src/commands/CommandIndex.cpp"
    "src/commands/CommandMatcher.cpp"
    "src/ir/codegen/CodeGenerator.cpp"
    "src/ir/codegen/ODBEngineInterface.cpp"
    "src/ir/codegen/DBPEngineInterface.cpp"
    "src/ir/semantic/ASTConverter.cpp"
    "src/ir/Codegen.cpp"
    "src/ir/Node.cpp"
    "src/ir/SemanticChecker.cpp"
    "src/parsers/db/Driver.cpp"
    "src/parsers/TargetLibParser.cpp")
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
# C4661: "no suitable definition provided for explicit template instantiation request"
#        Implementations of explicitly instantiated template classes exist in .cpp files
#        instead of header files, so this warning can be ignored.
target_compile_options (odb-compiler
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>
		$<$<CXX_COMPILER_ID:MSVC>:/wd4661>)
target_compile_features (odb-compiler
    PUBLIC cxx_std_17)
target_link_libraries (odb-compiler
    PUBLIC
        odb-sdk)
set_target_properties (odb-compiler
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_ARCHIVE_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_LIBRARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR}
        POSITION_INDEPENDENT_CODE ${ODBCOMPILER_PIC})

###############################################################################
# Executable parsing.
###############################################################################
include (ExternalProject)

ExternalProject_Add(
    lief
    URL https://github.com/lief-project/LIEF/archive/0.11.2.tar.gz
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/lief-prefix/install
        -DBUILD_SHARED_LIBS=OFF
        -DLIEF_PYTHON_API=OFF
        -DLIEF_C_API=OFF
        -DLIEF_EXAMPLES=OFF
        -DLIEF_USE_CCACHE=OFF
        -DLIEF_LOGGING=OFF
        -DLIEF_LOGGING_DEBUG=OFF
        -DLIEF_ENABLE_JSON=OFF
        -DLIEF_OAT=OFF
        -DLIEF_DEX=OFF
        -DLIEF_VDEX=OFF
        -DLIEF_ART=OFF
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/lief-prefix/install/lib/libLIEF.a
)
target_include_directories(odb-compiler PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/lief-prefix/install/include)
target_link_libraries(odb-compiler PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/lief-prefix/install/lib/libLIEF.a)
add_dependencies(odb-compiler lief)

###############################################################################
# LLVM stuff
###############################################################################

find_package (LLVM REQUIRED CONFIG)
find_package (LLD REQUIRED)

# This is apparently deprecated since 4 years
# https://lists.llvm.org/pipermail/llvm-dev/2017-November/119054.html
#
#llvm_map_components_to_libnames (llvm_libs <list of LLVM libs>)
#target_link_directories (odb-compiler PUBLIC ${LLVM_LIBRARY_DIRS})
#target_link_libraries (odb-compiler PUBLIC ${llvm_libs})

# This is how you're supposed to do it by reading LLVM-Config.cmake
# https://github.com/llvm-mirror/llvm/blob/master/cmake/modules/LLVM-Config.cmake#L67
if (${ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS})
    set (llvm_use_shared USE_SHARED)
endif()
llvm_config (odb-compiler ${llvm_use_shared} core bitwriter x86codegen aarch64codegen)

target_include_directories (odb-compiler PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions (odb-compiler PUBLIC ${LLVM_DEFINITIONS})

target_link_libraries (odb-compiler PRIVATE lldCOFF)

###############################################################################
# Unit tests
###############################################################################

if (${ODBCOMPILER_TESTS})
    add_executable (odbc_tests
        "tests/src/astpost/test_astpost_eliminate_bitwise_not_rhs.cpp"
        "tests/src/astpost/test_astpost_validate_udt_field_names.cpp"
        "tests/src/commands/test_cmd_matcher.cpp"
        "tests/src/harness/ParserTestHarness.cpp"
        "tests/src/matchers/AnnotatedSymbolEq.cpp"
        "tests/src/matchers/ArgListCountEq.cpp"
        "tests/src/matchers/BinaryOpEq.cpp"
        "tests/src/matchers/BlockStmntCountEq.cpp"
        "tests/src/matchers/CommandExprEq.cpp"
        "tests/src/matchers/CommandStmntEq.cpp"
        "tests/src/matchers/GotoEq.cpp"
        "tests/src/matchers/InitializerListCountEq.cpp"
        "tests/src/matchers/LiteralEq.cpp"
        "tests/src/matchers/ScopedAnnotatedSymbolEq.cpp"
        "tests/src/matchers/SymbolEq.cpp"
        "tests/src/matchers/UDTRefEq.cpp"
        "tests/src/matchers/UnaryOpEq.cpp"
        "tests/src/parser/test_db_parser_array_decl.cpp"
        "tests/src/parser/test_db_parser_arrays.cpp"
        "tests/src/parser/test_db_parser_assignment.cpp"
        "tests/src/parser/test_db_parser_command.cpp"
        "tests/src/parser/test_db_parser_conditional.cpp"
        "tests/src/parser/test_db_parser_constant.cpp"
        "tests/src/parser/test_db_parser_func_call.cpp"
        "tests/src/parser/test_db_parser_func_decl.cpp"
        "tests/src/parser/test_db_parser_goto.cpp"
        "tests/src/parser/test_db_parser_inc_dec.cpp"
        "tests/src/parser/test_db_parser_literal_bool.cpp"
        "tests/src/parser/test_db_parser_literal_complex.cpp"
        "tests/src/parser/test_db_parser_literal_float.cpp"
        "tests/src/parser/test_db_parser_literal_int.cpp"
        "tests/src/parser/test_db_parser_literal_quat.cpp"
        "tests/src/parser/test_db_parser_literal_string.cpp"
        "tests/src/parser/test_db_parser_location_info.cpp"
        "tests/src/parser/test_db_parser_loop_do.cpp"
        "tests/src/parser/test_db_parser_loop_for.cpp"
        "tests/src/parser/test_db_parser_loop_repeat.cpp"
        "tests/src/parser/test_db_parser_loop_while.cpp"
        "tests/src/parser/test_db_parser_op_precedence.cpp"
        "tests/src/parser/test_db_parser_remarks.cpp"
        "tests/src/parser/test_db_parser_select.cpp"
        "tests/src/parser/test_db_parser_subroutine.cpp"
        "tests/src/parser/test_db_parser_udt_var_decl.cpp"
        "tests/src/parser/test_db_parser_udt_fields.cpp"
        "tests/src/parser/test_db_parser_var_decl.cpp"
        "tests/src/parser/test_db_parser_var_decl_math.cpp"
        "tests/src/parser/test_db_parser_var_ref.cpp"
        "tests/src/parser/ASTParentConsistenciesChecker.cpp"
        "tests/src/test_SourceLocation.cpp"
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
	target_compile_options (odbc_tests
		PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:/bigobj>)
    set_target_properties (odbc_tests
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})
endif ()

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
