project ("odbi"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)

###############################################################################
# Configuration
###############################################################################

configure_file ("templates/config.hpp.in" "include/odbi/config.hpp")

###############################################################################
# Executable definition
###############################################################################

add_executable (odbi
    "src/main.cpp")
target_include_directories (odbi
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:$<$<AND:$<PLATFORM_ID:Windows>,$<NOT:$<BOOL:${HAVE_UNISTD_H}>>>:${PROJECT_SOURCE_DIR}/include/win32_unistd>>)
target_compile_options (odbi
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror -pedantic -Wno-unused-function -Wno-unused-parameter>)
set_target_properties (odbi
    PROPERTIES CXX_STANDARD 17)
target_link_libraries (odbi
    PRIVATE
        odb-compiler)
set_target_properties (odbi
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${ODB_RUNTIME_DIR})

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odbi
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
