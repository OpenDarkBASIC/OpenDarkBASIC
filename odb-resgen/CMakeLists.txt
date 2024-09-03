project ("odb-resgen"
    LANGUAGES C
    VERSION 0.0.1)

add_executable (odb-resgen
    "src/main.c")
target_compile_options(odb-resgen
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<C_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
include (ODBTargetProperties)
odb_target_properties (odb-resgen
    PROPERTIES
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})

install (
    TARGETS odb-resgen
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})

macro (odb_resgen_target name)
    set (odb_resgen_target_PARAM_OPTIONS)
    set (odb_resgen_target_PARAM_ONE_VALUE_KEYWORDS
        TARGET)
    set (odb_resgen_target_PARAM_MULTI_VALUE_KEYWORDS
        INPUT)
    cmake_parse_arguments (
        odb_resgen_target_arg
        "${odb_resgen_target_PARAM_OPTIONS}"
        "${odb_resgen_target_PARAM_ONE_VALUE_KEYWORDS}"
        "${odb_resgen_target_PARAM_MULTI_VALUE_KEYWORDS}"
        ${ARGN})

    if (NOT "${odb_resgen_target_arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message (FATAL_ERROR "odb_resgen_target <name> TARGET <winres|elf> INPUT <files...>")
    endif ()

    if (${odb_resgen_target_arg_TARGET} STREQUAL "winres")
        add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/odbres.rc
            COMMAND odb-resgen
            ARGS
                -t ${odb_resgen_target_arg_TARGET}
                -i ${odb_resgen_target_arg_INPUT}
                -o ${PROJECT_BINARY_DIR}/odbres.rc
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            DEPENDS
                odb-resgen
                ${odb_resgen_target_arg_INPUT}
            COMMENT "[odb-resgen] Generating resource strings for ${name}"
            VERBATIM)
        set (ODB_RESGEN_${name}_OUTPUTS ${PROJECT_BINARY_DIR}/odbres.rc)
    else ()
        add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/odbres
            COMMAND odb-resgen
            ARGS
                -t ${odb_resgen_target_arg_TARGET}
                -i ${odb_resgen_target_arg_INPUT}
                -o ${PROJECT_BINARY_DIR}/odbres
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            DEPENDS
                odb-resgen
                ${odb_resgen_target_arg_INPUT}
            COMMENT "[odb-resgen] Generating resource strings for ${name}"
            VERBATIM)

        add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/odbres.o
            COMMAND ${CMAKE_OBJCOPY}
            ARGS
                --input-target binary
                --output-target elf64-x86-64
                --binary-architecture i386:x86-64
                --rename-section .data=.odbres,CONTENTS,ALLOC,LOAD,READONLY,DATA
                --add-section .note.GNU-stack=/dev/null  # silence warnings about executable stack
                odbres odbres.o
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            DEPENDS ${PROJECT_BINARY_DIR}/odbres
            COMMENT "[odb-resgen] Compiling resource strings for ${name}"
            VERBATIM)
        set (ODB_RESGEN_${name}_OUTPUTS ${PROJECT_BINARY_DIR}/odbres.o)
    endif ()

    set (ODB_RESGEN_${name}_DEFINED TRUE)
endmacro ()
