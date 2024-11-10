project ("odb-asttool"
    LANGUAGES C
    VERSION 0.0.1)

option (ODBASTTOOL_WERROR "Treat compiler warnings as errors" ${ODB_WERROR})

add_executable (odb-asttool
    "include/odb-asttool/asttool.h"
    "include/odb-asttool/export.h"
    "src/graphviz.c"
    "src/gtest.c"
    "src/asttool.c")
# Only need the ast.h header from odb-compiler
get_target_property (COMPILER_INCLUDES odb-compiler INCLUDE_DIRECTORIES)
target_include_directories (odb-asttool
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${COMPILER_INCLUDES}>)
target_link_libraries (odb-asttool
    PRIVATE
        odb-util)
target_compile_options (odb-asttool
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<C_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
if (ODBASTOOL_WERROR)
    target_compile_options (odb-asttool
        PRIVATE
            $<$<C_COMPILER_ID:GNU>:-Werror>
            $<$<C_COMPILER_ID:Clang>:-Werror>
            $<$<C_COMPILER_ID:AppleClang>:-Werror>
            $<$<CXX_COMPILER_ID:GNU>:-Werror>
            $<$<CXX_COMPILER_ID:Clang>:-Werror>
            $<$<CXX_COMPILER_ID:AppleClang>:-Werror>)
endif ()
include (ODBTargetProperties)
odb_target_properties (odb-asttool
    PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR}
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR}
        INSTALL_RPATH ${ODB_INSTALL_LIBDIR})

install (
    TARGETS odb-asttool
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})

