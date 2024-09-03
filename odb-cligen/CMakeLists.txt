project ("odb-cligen"
    LANGUAGES C
    VERSION 0.0.1)

add_executable (odb-cligen
    "src/main.c")
target_compile_options(odb-cligen
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<C_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
include (ODBTargetProperties)
odb_target_properties (odb-cligen
    PROPERTIES
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})

install (
    TARGETS odb-cligen
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})

macro (odb_cligen_target name)
    set (odb_cligen_target_PARAM_OPTIONS)
    set (odb_cligen_target_PARAM_ONE_VALUE_KEYWORDS
        NAME
        INPUT
        OUTPUT)
    set (odb_cligen_target_PARAM_MULTI_VALUE_KEYWORDS)

    cmake_parse_arguments (
        odb_cligen_target_arg
        "${odb_cligen_target_PARAM_OPTIONS}"
        "${odb_cligen_target_PARAM_ONE_VALUE_KEYWORDS}"
        "${odb_cligen_target_PARAM_MULTI_VALUE_KEYWORDS}"
        ${ARGN})

    if (NOT "${odb_cligen_target_arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message (FATAL_ERROR "odb_cligen_target <name> <input> <output>")
    endif ()

    add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/odbres.rc
        COMMAND odb-cligen
        ARGS
            -i ${odb_cligen_target_arg_INPUT}
            -o ${odb_cligen_target_arg_OUTPUT}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS
            odb-cligen
            ${odb_cligen_target_arg_INPUT}
        COMMENT "[odb-cligen] Generating CLI dependencies ${name}"
        VERBATIM)

    set (ODB_CLIGEN_${name}_OUTPUTS ${odb_cligen_target_arg_OUTPUT})
    set (ODB_CLIGEN_${name}_DEFINED TRUE)
endmacro ()
