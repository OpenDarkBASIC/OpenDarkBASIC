project (argdefgen
    LANGUAGES C
    VERSION 1.0.0)

find_package (FLEX REQUIRED 2.6)
find_package (BISON REQUIRED 3.2)


# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/argdefgen")

bison_target (adg_parser
    "${PROJECT_SOURCE_DIR}/src/parser.y"
    "${PROJECT_BINARY_DIR}/src/parser.y.c"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/argdefgen/parser.y.h"
    COMPILE_FLAGS -Wcounterexamples)
flex_target (adg_scanner
    "${PROJECT_SOURCE_DIR}/src/scanner.lex"
    "${PROJECT_BINARY_DIR}/src/scanner.lex.c"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/argdefgen/scanner.lex.h")
add_flex_bison_dependency (adg_scanner adg_parser)

add_subdirectory ("templates")
add_custom_command (
    OUTPUT "${PROJECT_BINARY_DIR}/src/argparse.c"
    COMMAND argdefgen_gen_templates
    ARGS "${PROJECT_BINARY_DIR}/src/argparse.c"
         agd_argparse_cpp_preamble "${PROJECT_SOURCE_DIR}/templates/cpp_preamble.cpp"
         agd_argparse_cpp_postamble "${PROJECT_SOURCE_DIR}/templates/cpp_postamble.cpp"
    DEPENDS argdefgen_gen_templates
            "${PROJECT_SOURCE_DIR}/templates/cpp_preamble.cpp"
            "${PROJECT_SOURCE_DIR}/templates/cpp_postamble.cpp"
    COMMENT "Generating src/argparse.c"
    VERBATIM)

add_executable (argdefgen
    "src/action.c"
    "src/driver.c"
    "src/gen_cpp.c"
    "src/main.c"
    "src/node.c"
    "src/str.c"
    "${PROJECT_BINARY_DIR}/src/argparse.c"
    ${BISON_adg_parser_OUTPUTS}
    ${FLEX_adg_scanner_OUTPUTS})
target_include_directories (argdefgen
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

###############################################################################
#
###############################################################################

macro (argdefgen_target Name Input Output)
    set (argdefgen_target_PARAM_OPTIONS)
    set (argdefgen_target_PARAM_ONE_VALUE_KEYWORDS
        DEFINES_FILE
        DEPGRAPH
        AST
        COMPILE_FLAGS)
    set (argdefgen_target_PARAM_MULTI_VALUE_KEYWORDS)

    cmake_parse_arguments (
        argdefgen_target_arg
        "${argdefgen_target_PARAM_OPTIONS}"
        "${argdefgen_target_PARAM_ONE_VALUE_KEYWORDS}"
        "${argdefgen_target_PARAM_MULTI_VALUE_KEYWORDS}"
        ${ARGN})

    if (NOT "${argdefgen_target_arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message (FATAL_ERROR "argdefgen_target(<Name> <Input> <Output> [DEFINES_FILE <file>] [COMPILE_FLAGS <string>] [DEPGRAPH <file>] [AST <file>]")
    endif ()

    set (_adg_WORKING_DIR "${CMAKE_CURRENT_BINARY_DIR}")

    set (_adg_INPUT "${Input}")
    if (NOT IS_ABSOLUTE "${_adg_INPUT}")
        set (_adg_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_adg_INPUT}")
    endif ()

    set (_adg_OUTPUT "${Output}")
    if (NOT IS_ABSOLUTE ${_adg_OUTPUT})
        set (_adg_OUTPUT "${_adg_WORKING_DIR}/${_adg_OUTPUT}")
    endif ()
    set (_adg_TARGET_OUTPUTS "${_adg_OUTPUT}")

    set (_adg_EXE_OPTS "")
    if (NOT "${argdefgen_target_arg_COMPILE_FLAGS}" STREQUAL "")
        set (_adg_EXE_OPTS  "${argdefgen_target_arg_COMPILE_FLAGS}")
        separate_arguments (_adg_EXE_OPTS)
    endif ()

    set (_adg_OUTPUT_HEADER "")
    if (NOT "${argdefgen_target_arg_DEFINES_FILE}" STREQUAL "")
        set (_adg_OUTPUT_HEADER "${argdefgen_target_arg_DEFINES_FILE}")
        if (NOT IS_ABSOLUTE "${_adg_OUTPUT_HEADER}")
            set (_adg_OUTPUT_HEADER "${_adg_WORKING_DIR}/${_adg_OUTPUT_HEADER}")
        endif ()
        list (APPEND _adg_TARGET_OUTPUTS "${_adg_OUTPUT_HEADER}")
        list (APPEND _adg_EXE_OPTS --header ${_adg_OUTPUT_HEADER})
    endif ()

    set (_adg_OUTPUT_DEPGRAPH "")
    if (NOT "${argdefgen_target_arg_DEPGRAPH}" STREQUAL "")
        set (_adg_OUTPUT_DEPGRAPH "${argdefgen_target_arg_DEPGRAPH}")
        if (NOT IS_ABSOLUTE "${_adg_OUTPUT_DEPGRAPH}")
            set (_adg_OUTPUT_DEPGRAPH "${_adg_WORKING_DIR}/${_adg_OUTPUT_DEPGRAPH}")
        endif ()
        list (APPEND _adg_EXE_OPTS --depgraph ${_adg_OUTPUT_DEPGRAPH})
    endif ()

    set (_adg_OUTPUT_AST "")
    if (NOT "${argdefgen_target_arg_AST}" STREQUAL "")
        set (_adg_OUTPUT_AST "${argdefgen_target_arg_AST}")
        if (NOT IS_ABSOLUTE "${_adg_OUTPUT_AST}")
            set (_adg_OUTPUT_AST "${_adg_WORKING_DIR}/${_adg_OUTPUT_AST}")
        endif ()
        list (APPEND _adg_EXE_OPTS --ast ${_adg_OUTPUT_AST})
    endif ()

    add_custom_command (OUTPUT ${_adg_OUTPUT} ${_adg_OUTPUT_HEADER}
        COMMAND argdefgen
        ARGS ${_adg_EXE_OPTS} --source ${_adg_OUTPUT} ${_adg_INPUT}
        WORKING_DIRECTORY ${_adg_WORKING_DIR}
        MAIN_DEPENDENCY ${_adg_INPUT}
        DEPENDS argdefgen
        COMMENT "[argdefgen][${Name}] Generating command line argument parser"
        VERBATIM)

    set (argdefgen_${Name}_DEFINED TRUE)
    set (argdefgen_${Name}_OUTPUTS ${_adg_TARGET_OUTPUTS})
    set (argdefgen_${Name}_OUTPUT_HEADER ${_adg_OUTPUT_HEADER})
    set (argdefgen_${Name}_INPUT ${_adg_INPUT})
    set (argdefgen_${Name}_COMPILE_FLAGS ${_adg_EXE_OPTS})

    unset (_adg_WORKING_DIR)
    unset (_adg_INPUT)
    unset (_adg_OUTPUT)
    unset (_adg_TARGET_OUTPUTS)
    unset (_adg_EXE_OPTS)
    unset (_adg_OUTPUT_HEADER)
    unset (_adg_OUTPUT_DEPGRAPH)
    unset (_adg_OUTPUT_AST)
endmacro ()
