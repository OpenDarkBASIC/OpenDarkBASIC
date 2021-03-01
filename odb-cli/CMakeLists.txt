include (GNUInstallDirs)
add_subdirectory ("argdefgen")

project ("odbc"
    LANGUAGES CXX
    VERSION 0.0.1)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-cli")

add_custom_command (
    OUTPUT "${PROJECT_BINARY_DIR}/src/Actions.argdef.cpp"
           "${PROJECT_BINARY_DIR}/include/odb-cli/Actions.argdef.hpp"
    COMMAND argdefgen
    ARGS --argdef "src/Actions.argdef"
         --source "${PROJECT_BINARY_DIR}/src/Actions.argdef.cpp"
         --header "${PROJECT_BINARY_DIR}/include/odb-cli/Actions.argdef.hpp"
         --ast "${PROJECT_BINARY_DIR}/Actions.argdef.ast.dot"
         --depgraph "${PROJECT_BINARY_DIR}/Actions.argdef.depgraph.dot"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    MAIN_DEPENDENCY "src/Actions.argdef"
    DEPENDS argdefgen
    COMMENT "Generating argument table from argdef"
    VERBATIM)

###############################################################################
# Executable
###############################################################################

add_executable (odbc
    "${PROJECT_BINARY_DIR}/src/Actions.argdef.cpp"
    "src/AST.cpp"
    "src/Banner.cpp"
    "src/BuildInfo.cpp"
    "src/Codegen.cpp"
    "src/Commands.cpp"
    "src/Log.cpp"
    "src/SDK.cpp"
    "src/main.cpp")
target_include_directories (odbc
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries (odbc
    PRIVATE
        odb-compiler)
set_target_properties (odbc
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR}
        INSTALL_RPATH ${CMAKE_INSTALL_LIBDIR})

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odbc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
