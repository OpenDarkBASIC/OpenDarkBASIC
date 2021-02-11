project ("odbc"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)

###############################################################################
# Executable
###############################################################################

add_executable (odbc
    "src/Actions.cpp"
    "src/AST.cpp"
    "src/Banner.cpp"
    "src/BuildInfo.cpp"
    "src/Codegen.cpp"
    "src/Commands.cpp"
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
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odbc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
