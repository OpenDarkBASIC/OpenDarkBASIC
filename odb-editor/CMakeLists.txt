list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

project (odb-editor
    LANGUAGES C
    VERSION 0.0.1)

option (ODBEDITOR_WERROR "Treat compiler warnings as errors" ${ODB_WERROR})

find_package (GTK4 REQUIRED COMPONENTS
    gtk glib cairo pango harfbuzz gdk-pixbuf graphene)

add_subdirectory ("thirdparty/libxml-cmake")
add_subdirectory ("thirdparty/gtksourceview-cmake")

gtk_compile_resource (
    "res/odbeditor.gresource.xml"
    "src/odbeditor.gresource.c")

add_executable (odb-editor
    "res/odbeditor.gresource.xml"

    "${PROJECT_BINARY_DIR}/src/odbeditor.gresource.c"
    #"src/file_browser.c"
    "src/main.c")
target_include_directories (odb-editor
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_compile_options (odb-editor
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<C_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive -fvisibility=hidden>)
if (ODBEDITOR_WERROR)
    target_compile_options (odb-editor
        PRIVATE
            $<$<C_COMPILER_ID:GNU>:-Werror>
            $<$<C_COMPILER_ID:Clang>:-Werror>
            $<$<C_COMPILER_ID:AppleClang>:-Werror>
            $<$<CXX_COMPILER_ID:GNU>:-Werror>
            $<$<CXX_COMPILER_ID:Clang>:-Werror>
            $<$<CXX_COMPILER_ID:AppleClang>:-Werror>)
endif ()
target_link_libraries (odb-editor
    PRIVATE
        odb-util
        odb-compiler
        gtksourceview
        GTK4::glib
        GTK4::cairo
        GTK4::pango
        GTK4::harfbuzz
        GTK4::gdk-pixbuf
        GTK4::graphene
        GTK4::gtk)
include (ODBTargetProperties)
odb_target_properties (odb-editor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR}
    MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
    RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})

if (WIN32 OR CYGWIN)
    add_custom_command (TARGET odb-editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GTK4_RUNTIMES} ${ODB_BUILD_BINDIR}
        COMMENT "Copying GTK4 DLLs to ${ODB_BUILD_BINDIR}"
        VERBATIM)
endif ()

install (
    TARGETS odb-editor
    DESTINATION ${ODB_INSTALL_BINDIR})
