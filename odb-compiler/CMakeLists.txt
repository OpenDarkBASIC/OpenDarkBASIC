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
option (ODBCOMPILER_UPDATE_BUILDINFO "If true the updated build number is written to a header, which causes the library to recompile build_info.c. This can get annoying for developers, so disable this if you are annoyed." ${RELEASE_FEATURE})
option (ODBCOMPILER_AST_SANITY_CHECK "Enable checking AST integrity after semantic checks run" ${DEBUG_FEATURE})
option (ODBCOMPILER_DOT_EXPORT "Enable functions for dumping AST to DOT format" ON)
option (ODBCOMPILER_IR_SANITY_CHECK "Enable running llvm::verifyFunction() on generated IR" ${DEBUG_FEATURE})
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

string (TOUPPER ${CMAKE_SYSTEM_NAME} ODBCOMPILER_PLATFORM)

configure_file ("templates/config.h.in" "include/odb-compiler/config.h")

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
set (BULIDINFOGENERATED_HEADER "${PROJECT_BINARY_DIR}/include/odb-compiler/build_info_gen.h")
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
            "templates/build_info.c.in"
            odbcompiler_build_info_generated
    VERBATIM)

configure_file ("templates/build_info.c.in" "src/build_info.c")

###############################################################################
# Parsers
###############################################################################

find_package (FLEX REQUIRED 2.6)
find_package (BISON REQUIRED 3.8)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parser")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-compiler/parser")

if (WIN32)
    set (FLEX_COMPILE_FLAGS --wincompat)
endif ()
if (ODBCOMPILER_BISON_COUNTER_EXAMPLES)
    set (BISON_COMPILE_FLAGS "-Wcounterexamples")
endif ()
if (ODBCOMPILER_VERBOSE_BISON)
    set (BISON_VERBOSE "VERBOSE")
endif ()

bison_target (db-parser
    "${PROJECT_SOURCE_DIR}/src/parser/db_parser.y"
    "${PROJECT_BINARY_DIR}/src/parser/db_parser.y.c"
    ${BISON_VERBOSE}
    COMPILE_FLAGS "${BISON_COMPILE_FLAGS}"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odb-compiler/parser/db_parser.y.h")
flex_target (db-scanner
    "${PROJECT_SOURCE_DIR}/src/parser/db_scanner.lex"
    "${PROJECT_BINARY_DIR}/src/parser/db_scanner.lex.c"
    COMPILE_FLAGS ${FLEX_COMPILE_FLAGS}
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odb-compiler/parser/db_scanner.lex.h")
add_flex_bison_dependency (db-scanner db-parser)

if (ODBCOMPILER_VERBOSE_BISON)
    set_property (
        SOURCE ${BISON_db-parser_OUTPUTS}
        PROPERTY COMPILE_DEFINITIONS YYDEBUG)
endif ()

gperf_target (db-keyword
    "${PROJECT_SOURCE_DIR}/src/parser/db_keyword.gperf"
    "${PROJECT_BINARY_DIR}/src/parser/db_keyword.gperf.c")
set_source_files_properties (${Gperf_db-keyword_OUTPUTS}
    PROPERTIES COMPILE_FLAGS
        $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-field-initializers>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-missing-field-initializers>)
#XXX: this is broken
add_gperf_bison_dependency (db-keyword db-parser)

add_custom_target (db-parser DEPENDS ${BISON_db-parser_OUTPUTS})

###############################################################################
# Library definition
###############################################################################

add_library (odb-compiler ${ODBCOMPILER_LIB_TYPE}
    "templates/config.h.in"
    "include/odb-compiler/build_info.h"
    "templates/build_info.c.in"
    "${PROJECT_BINARY_DIR}/include/odb-compiler/config.h"
    "${BULIDINFOGENERATED_HEADER}"
    "${PROJECT_BINARY_DIR}/src/build_info.c"

    "include/odb-compiler/sdk/sdk_type.h"
    "include/odb-compiler/sdk/cmd_cache.h"
    "include/odb-compiler/sdk/used_cmds.h"
    "include/odb-compiler/sdk/cmd_list.h"
    "include/odb-compiler/sdk/plugin_list.h"
    "include/odb-compiler/sdk/type.h"
    "src/sdk/cmd_cache.c"
    "src/sdk/used_cmds.c"
    "src/sdk/cmd_list.c"
    "src/sdk/cmd_loader.cpp"
    "src/sdk/cmd_loader_dbpro.cpp"
    "src/sdk/cmd_loader_odb.cpp"
    "src/sdk/plugin_list.c"
    "src/sdk/type.c"
    "src/sdk/sdk_type.c"

    "include/odb-compiler/parser/db_keyword.h"
    "include/odb-compiler/parser/db_parser.h"
    "include/odb-compiler/parser/db_source.h"
    "src/parser/db_keyword.gperf"
    "src/parser/db_scanner.lex"
    "src/parser/db_parser.y"
    "src/parser/db_parser.c"
    "src/parser/db_source.c"
    "${Gperf_db-keyword_OUTPUTS}"
    "${BISON_db-parser_OUTPUTS}"
    "${FLEX_db-scanner_OUTPUTS}"
   
    "include/odb-compiler/ast/ast.h"
    "include/odb-compiler/ast/ast_ops.h"
    "include/odb-compiler/ast/ast_export.h"
    "src/ast/ast.c"
    "src/ast/ast_ops.c"
    "src/ast/ast_export.c"
    #"src/ast/Annotation.cpp"
    #"src/ast/ArgList.cpp"
    #"src/ast/ArrayDecl.cpp"
    #"src/ast/ArrayUndim.cpp"
    #"src/ast/ArrayRef.cpp"
    #"src/ast/Assignment.cpp"
    #"src/ast/BinaryOp.cpp"
    #"src/ast/Block.cpp"
    #"src/ast/CommandExpr.cpp"
    #"src/ast/CommandStmnt.cpp"
    #"src/ast/Conditional.cpp"
    #"src/ast/ConstDecl.cpp"
    #"src/ast/Exit.cpp"
    #"src/ast/Exporters_DOT.cpp"
    #"src/ast/Exporters_JSON.cpp"
    #"src/ast/Expression.cpp"
    #"src/ast/FuncArgList.cpp"
    #"src/ast/FuncCall.cpp"
    #"src/ast/FuncDecl.cpp"
    #"src/ast/Goto.cpp"
    #"src/ast/Identifier.cpp"
    #"src/ast/ImplicitCast.cpp"
    #"src/ast/InitializerList.cpp"
    #"src/ast/Label.cpp"
    #"src/ast/Literal.cpp"
    #"src/ast/Loop.cpp"
    #"src/ast/LValue.cpp"
    #"src/ast/Node.cpp"
    #"src/ast/Operator.cpp"
    #"src/ast/ParentMap.cpp"
    #"src/ast/Program.cpp"
    #"src/ast/Scope.cpp"
    #"src/ast/ScopedIdentifier.cpp"
    #"src/ast/SelectCase.cpp"
    #"src/ast/SourceLocation.cpp"
    #"src/ast/Statement.cpp"
    #"src/ast/Subroutine.cpp"
    #"src/ast/Type.cpp"
    #"src/ast/UDTDecl.cpp"
    #"src/ast/UDTField.cpp"
    #"src/ast/UnaryOp.cpp"
    #"src/ast/VarDecl.cpp"
    #"src/ast/Variable.cpp"
    #"src/ast/VariableScope.cpp"
    #"src/ast/VarRef.cpp"
    #"src/ast/Visitor.cpp"
   
    "include/odb-compiler/semantic/semantic.h"
    "src/semantic/semantic.c"
    "src/semantic/expand_constant_declarations.c"
    "src/semantic/loop_exit.c"
    "src/semantic/resolve_cmd_overloads.c"
    "src/semantic/type_check.c"
    "src/semantic/type_check_assignment.c"
    "src/semantic/type_check_binop_symmetric.c"
    "src/semantic/type_check_binop_pow.c"
    "src/semantic/type_check_casts.c"
    #"src/astpost/EliminateBitwiseNotRHS.cpp"
    #"src/astpost/EliminateBitwiseNotRHS.cpp"
    #"src/astpost/EnforceSingleDefaultCase.cpp"
    #"src/astpost/Process.cpp"
    #"src/astpost/ResolveAndCheckTypes.cpp"
    #"src/astpost/ResolveLabels.cpp"
    #"src/astpost/ValidateUDTFieldNames.cpp"
    
    "include/odb-compiler/codegen/ir.h"
    "include/odb-compiler/codegen/target.h"
    "src/codegen/target.c"
    "src/codegen/ir_internal.hpp"
    "src/codegen/ir.cpp"
    "src/codegen/ir_compile.cpp"
    "src/codegen/ir_optimize.cpp"
    "src/codegen/ir_runtime.cpp"
    #"src/codegen/internal/CodeGenerator.cpp"
    #"src/codegen/internal/ODBEngineInterface.cpp"
    #"src/codegen/internal/DBPEngineInterface.cpp"
    
    "include/odb-compiler/link/link.h"
    "src/link/link.cpp") 
target_include_directories (odb-compiler
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
    #PRIVATE
    #    $<BUILD_INTERFACE:$<$<AND:$<PLATFORM_ID:Windows>,$<NOT:$<BOOL:${HAVE_UNISTD_H}>>>:${PROJECT_SOURCE_DIR}/include/win32_unistd>>)
target_compile_definitions (odb-compiler
    PRIVATE
        ODBCOMPILER_BUILDING)
target_compile_options (odb-compiler
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<C_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive -fvisibility=hidden>)
target_link_libraries (odb-compiler
    PUBLIC
        odb-sdk)
include (ODBTargetProperties)
odb_target_properties (odb-compiler
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR}
        POSITION_INDEPENDENT_CODE ${ODBCOMPILER_PIC})

###############################################################################
# Dependencies.
###############################################################################

include (FetchContent)

# LIEF is required for loading commands from shared libraries
set (LIEF_ART OFF CACHE BOOL "")
set (LIEF_C_API OFF CACHE BOOL "")
set (LIEF_DEX OFF CACHE BOOL "")
set (LIEF_ENABLE_JSON OFF CACHE BOOL "")
set (LIEF_EXAMPLES OFF CACHE BOOL "")
set (LIEF_FROZEN_ENABLED OFF CACHE BOOL "")
set (LIEF_INSTALL OFF CACHE BOOL "")
set (LIEF_LOGGING OFF CACHE BOOL "")
set (LIEF_LOGGING_DEBUG OFF CACHE BOOL "")
set (LIEF_OAT OFF CACHE BOOL "")
set (LIEF_PYTHON_API OFF CACHE BOOL "")
set (LIEF_USE_CCACHE OFF CACHE BOOL "")
set (LIEF_VDEX OFF CACHE BOOL "")
FetchContent_Declare (
    lief
    URL https://github.com/lief-project/LIEF/archive/refs/tags/0.14.1.tar.gz
    URL_HASH SHA256=92916dcb3178353d863aef4f409186889983c56e025b774741d5316a72ec3a7d
)
FetchContent_MakeAvailable (lief)
target_compile_options (LIB_LIEF
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<COMPILE_LANGUAGE:CXX>:-Wno-overloaded-virtual -Wno-unused-parameter -Wno-redundant-move>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<COMPILE_LANGUAGE:CXX>:-Wno-overloaded-virtual -Wno-unused-parameter -Wno-redundant-move>>
        $<$<CXX_COMPILER_ID:AppleClang>:$<$<COMPILE_LANGUAGE:CXX>:-Wno-overloaded-virtual -Wno-unused-parameter -Wno-redundant-move>>)
target_link_libraries (odb-compiler PRIVATE LIB_LIEF)

# reproc

#set(REPROC++ ON)
#set(REPROC_OBJECT_LIBRARIES ON)
#FetchContent_Declare (
#    reproc
#    URL https://github.com/DaanDeMeyer/reproc/archive/refs/tags/v14.2.4.tar.gz
#)
#FetchContent_MakeAvailable (reproc)

#set_property (TARGET reproc PROPERTY POSITION_INDEPENDENT_CODE ON)
#set_property (TARGET reproc++ PROPERTY POSITION_INDEPENDENT_CODE ON)

#target_link_libraries (odb-compiler PRIVATE reproc++)

###############################################################################
# LLVM and LLD
###############################################################################

find_package (LLVM REQUIRED CONFIG)
find_package (LLD REQUIRED CONFIG)

set (ODBCOMPILER_LLVM_LIBS
    Core
    BitWriter
    AArch64AsmParser
    AArch64CodeGen
    ARMAsmParser
    ARMCodeGen
    WebAssemblyAsmParser
    WebAssemblyCodeGen
    X86AsmParser
    X86CodeGen)

# These are the LLD libraries we need
set (ODBCOMPILER_LLD_LIBS
    lldCOFF
    lldELF
    lldMachO
    lldCommon)

if (${ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS})
    message (STATUS "Using shared LLVM libs")
    # Each LLD target is actually just a list of LLVM libraries. The problem is,
    # they are hardcoded and don't respect USE_SHARED, which results in us linking
    # both the static AND shared versions of LLVM libraries if we enable
    # ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS. For this reason, extract all
    # dependencies of the LLD targets and append them to the LLVM libs list.
    foreach (target ${ODBCOMPILER_LLD_LIBS})
        get_target_property (deps ${target} INTERFACE_LINK_LIBRARIES)
        set_property (TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES)
        list (APPEND ODBCOMPILER_LLVM_LIBS "${deps}")
    endforeach ()
    # lldCOFF, lldELF, etc. depend on lldCommon. Remove these
    list (REMOVE_ITEM ODBCOMPILER_LLVM_LIBS lldCommon)
    # Remove the LLVM prefix from each item, as these get appended again by llvm_config()
    list (TRANSFORM ODBCOMPILER_LLVM_LIBS REPLACE "LLVM" "")
    list (REMOVE_DUPLICATES ODBCOMPILER_LLVM_LIBS)

    # This is apparently deprecated since 4 years
    # https://lists.llvm.org/pipermail/llvm-dev/2017-November/119054.html
    #
    #llvm_map_components_to_libnames (llvm_libs <list of LLVM libs>)
    #target_link_directories (odb-compiler PUBLIC ${LLVM_LIBRARY_DIRS})
    #target_link_libraries (odb-compiler PUBLIC ${llvm_libs})
    #
    # This is how you're supposed to do it by reading LLVM-Config.cmake
    # https://github.com/llvm-mirror/llvm/blob/master/cmake/modules/LLVM-Config.cmake#L67
    llvm_config (odb-compiler USE_SHARED "${ODBCOMPILER_LLVM_LIBS}")
endif()
target_link_libraries (odb-compiler PRIVATE ${ODBCOMPILER_LLD_LIBS})

target_include_directories (odb-compiler
    PRIVATE 
        ${LLVM_INCLUDE_DIRS}
        ${LLD_INCLUDE_DIRS})
target_compile_definitions (odb-compiler PRIVATE ${LLVM_DEFINITIONS})

###############################################################################
# Unit tests
###############################################################################

if (ODBCOMPILER_TESTS)
    target_sources (odb-tests PRIVATE
        #"tests/src/ast/test_ast_treeiterator.cpp"
        #"tests/src/astpost/test_astpost_eliminate_bitwise_not_rhs.cpp"
        #"tests/src/astpost/test_astpost_resolve_labels.cpp"
        #"tests/src/astpost/test_astpost_validate_udt_field_names.cpp"
        #"tests/src/commands/test_cmd_matcher.cpp"
        #"tests/src/harness/ParserTestHarness.cpp"
        #"tests/src/matchers/ArgListCountEq.cpp"
        #"tests/src/matchers/ArrayDeclEq.cpp"
        #"tests/src/matchers/BinaryOpEq.cpp"
        #"tests/src/matchers/BlockStmntCountEq.cpp"
        #"tests/src/matchers/CommandExprEq.cpp"
        #"tests/src/matchers/CommandStmntEq.cpp"
        #"tests/src/matchers/GotoEq.cpp"
        #"tests/src/matchers/IdentifierEq.cpp"
        #"tests/src/matchers/InitializerListCountEq.cpp"
        #"tests/src/matchers/LiteralEq.cpp"
        #"tests/src/matchers/ScopedIdentifierEq.cpp"
        #"tests/src/matchers/SubCallEq.cpp"
        #"tests/src/matchers/UnaryOpEq.cpp"
        #"tests/src/matchers/VarDeclEq.cpp"
        #"tests/src/parser/test_db_parser_array_decl.cpp"
        #"tests/src/parser/test_db_parser_arrays.cpp"
        #"tests/src/parser/test_db_parser_assignment.cpp"
        #"tests/src/parser/test_db_parser_command.cpp"
        #"tests/src/parser/test_db_parser_conditional.cpp"
        #"tests/src/parser/test_db_parser_constant.cpp"
        #"tests/src/parser/test_db_parser_func_call.cpp"
        #"tests/src/parser/test_db_parser_func_decl.cpp"
        #"tests/src/parser/test_db_parser_goto.cpp"
        #"tests/src/parser/test_db_parser_inc_dec.cpp"
        #"tests/src/parser/test_db_parser_literal_bool.cpp"
        #"tests/src/parser/test_db_parser_literal_complex.cpp"
        #"tests/src/parser/test_db_parser_literal_float.cpp"
        #"tests/src/parser/test_db_parser_literal_int.cpp"
        #"tests/src/parser/test_db_parser_literal_quat.cpp"
        #"tests/src/parser/test_db_parser_literal_string.cpp"
        #"tests/src/parser/test_db_parser_location_info.cpp"
        #"tests/src/parser/test_db_parser_loop_do.cpp"
        #"tests/src/parser/test_db_parser_loop_for.cpp"
        #"tests/src/parser/test_db_parser_loop_repeat.cpp"
        #"tests/src/parser/test_db_parser_loop_while.cpp"
        #"tests/src/parser/test_db_parser_op_precedence.cpp"
        #"tests/src/parser/test_db_parser_remarks.cpp"
        #"tests/src/parser/test_db_parser_select.cpp"
        #"tests/src/parser/test_db_parser_subroutine.cpp"
        #"tests/src/parser/test_db_parser_udt_var_decl.cpp"
        #"tests/src/parser/test_db_parser_udt_fields.cpp"
        #"tests/src/parser/test_db_parser_var_decl.cpp"
        #"tests/src/parser/test_db_parser_var_decl_math.cpp"
        #"tests/src/parser/test_db_parser_var_ref.cpp"
        #"tests/src/test_SourceLocation.cpp"

        "tests/include/odb-compiler/tests/DBParserHelper.hpp"
        "tests/src/DBParserHelper.cpp"

        "tests/src/util/cmd_list.cpp"

        "tests/src/parser/boolean_literal.cpp"
        "tests/src/parser/integer_literal.cpp"
        "tests/src/parser/negative_integer_literal.cpp"
        "tests/src/parser/conditionals.cpp"
        "tests/src/parser/conditionals_errors.cpp"
        "tests/src/parser/command.cpp"
        "tests/src/parser/command_matcher.cpp"
        "tests/src/parser/loop_for.cpp"
        "tests/src/parser/loop_for_warnings.cpp"
        "tests/src/parser/loop_for_errors.cpp"
        "tests/src/parser/remarks.cpp"
        "tests/src/parser/remarks_errors.cpp"

        "tests/src/semantic/conditionals_warnings.cpp"
        "tests/src/semantic/initialize_undeclared_variables.cpp"
        "tests/src/semantic/loop_for_warnings.cpp"
        "tests/src/semantic/loop_for_errors.cpp"
        "tests/src/semantic/loop_exit.cpp"
        "tests/src/semantic/loop_exit_errors.cpp"
        "tests/src/semantic/loop_exit_for.cpp"
        "tests/src/semantic/loop_exit_for_errors.cpp"
        "tests/src/semantic/type_check_assignment.cpp"
        "tests/src/semantic/type_check_assignment_warnings.cpp"
        "tests/src/semantic/type_check_assignment_errors.cpp"
        "tests/src/semantic/type_check_binop_pow.cpp"
        "tests/src/semantic/type_check_binop_pow_warnings.cpp"
        "tests/src/semantic/type_check_binop_pow_errors.cpp"
        "tests/src/semantic/resolve_cmd_overloads.cpp"
        "tests/src/semantic/resolve_cmd_overloads_warnings.cpp"
        "tests/src/semantic/resolve_cmd_overloads_errors.cpp")
    target_link_libraries (odb-tests PRIVATE odb-compiler)
    target_include_directories (odb-tests
        PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/include>)

    # Continuous integration test cases are written using DBA source files
    # with associated expected outputs. odb-cigen is a program that converts
    # these files into GoogleTest test cases. Each subdirectory in the ci/
    # folder must be listed here explicitly in order to generate the test
    # cases
    set (CIGEN_SUITES
        "binop-precedence"
        "do-loop"
        "for-loop"
        "select-case")
    add_executable (odb-cigen
        "tests/src/ci/cigen.c")
    set (CIGEN_OUTPUTS)
    foreach (SUITE ${CIGEN_SUITES})
        string (REPLACE "-" "_" SUITE_NAME ${SUITE})
        file (GLOB_RECURSE CI_DBA_SOURCES "${PROJECT_SOURCE_DIR}/tests/src/ci/${SUITE}/*.dba")
        file (GLOB_RECURSE CI_RESULTS "${PROJECT_SOURCE_DIR}/tests/src/ci/${SUITE}/*.out")
        file (GLOB_RECURSE CI_ODB_RESULTS "${PROJECT_SOURCE_DIR}/tests/src/ci/${SUITE}/*.odbout")
        set (CIGEN_INPUTS ${CI_DBA_SOURCES} ${CI_RESULTS} ${CI_ODB_RESULTS})
        set (CIGEN_OUTPUT "${PROJECT_BINARY_DIR}/tests/src/ci/${SUITE_NAME}.cpp")
        add_custom_command (
            OUTPUT ${CIGEN_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/tests/src/ci"
            COMMAND odb-cigen
            ARGS
                --suite "odbcompiler_ci_${SUITE_NAME}"
                -i ${CIGEN_INPUTS}
                -o ${CIGEN_OUTPUT}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            DEPENDS
                odb-cigen
                ${CIGEN_INPUTS}
            COMMENT "[odb-cigen] Generating CI test cases for ${SUITE_NAME}"
            VERBATIM)
        target_sources (odb-tests PRIVATE ${CIGEN_OUTPUT})
        list (APPEND CIGEN_OUTPUTS ${CIGEN_OUTPUT})
    endforeach ()
    # There is an issue in cmake where the output of add_custom_command is not
    # added to a target's DEPENDS if target_sources() is used. We work around
    # this by creating a custom target that is a dependency of odb-tests,
    # which in turn DEPENDS on the outputs of odb-cigen.
    add_custom_target (odb-tests-cigen ALL DEPENDS ${CIGEN_OUTPUTS})
    add_dependencies (odb-tests odb-tests-cigen)
endif ()

###############################################################################
# Installation
###############################################################################

install (
    TARGETS
        odb-compiler
    EXPORT OpenDarkBASICConfig
    ARCHIVE DESTINATION ${ODB_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${ODB_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})
install (
    DIRECTORY "include/odb-compiler"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-compiler"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (
    EXPORT OpenDarkBASICConfig
    DESTINATION "share/odb/cmake")
