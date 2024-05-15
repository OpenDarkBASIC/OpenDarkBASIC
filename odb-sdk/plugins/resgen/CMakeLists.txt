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
set_target_properties (odb-resgen
    PROPERTIES
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})
target_link_libraries (odb-resgen PRIVATE odb-sdk)

install (
    TARGETS odb-resgen
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})

macro (odb_resgen_target name)
    set (odb_resgen_target_PARAM_OPTIONS)
    set (odb_resgen_target_PARAM_ONE_VALUE_KEYWORDS
        PLATFORM)
    set (odb_resgen_target_PARAM_MULTI_VALUE_KEYWORDS
        INPUT)
    cmake_parse_arguments (
        odb_resgen_target_arg
        "${odb_resgen_target_PARAM_OPTIONS}"
        "${odb_resgen_target_PARAM_ONE_VALUE_KEYWORDS}"
        "${odb_resgen_target_PARAM_MULTI_VALUE_KEYWORDS}"
        ${ARGN})

    if (NOT "${odb_resgen_target_arg_UNPARSED_ARGUMENTS}" STREQUAL "")
        message (FATAL_ERROR "odb_resgen_target <name> PLATFORM <Windows|Linux> INPUT <files...>")
    endif ()

    add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/typeinfo
        COMMAND odb-resgen ${PROJECT_BINARY_DIR}/typeinfo
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS odb-resgen
        COMMENT "[odb-resgen] Generating resource strings for ${name}"
        VERBATIM)

    add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/typeinfo.o
        COMMAND objcopy
        ARGS --input-target binary
             --output-target elf64-x86-64
             --binary-architecture i386:x86-64
             --rename-section .data=.odbtypeinfo,CONTENTS,ALLOC,LOAD,READONLY,DATA
             typeinfo
             typeinfo.o
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${PROJECT_BINARY_DIR}/typeinfo
        COMMENT "[odb-resgen] Compiling resource strings for ${name}"
        VERBATIM)

    set (ODB_RESGEN_${name}_DEFINED TRUE)
    set (ODB_RESGEN_${name}_OUTPUTS ${PROJECT_BINARY_DIR}/typeinfo.o)
endmacro ()
