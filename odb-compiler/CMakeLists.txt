project ("odbc"
    LANGUAGES CXX
    VERSION 0.0.1)

include (GNUInstallDirs)
include (TestVisibilityMacros)
include (ConfigureMSVCRuntime)

###############################################################################
# Executable definition
###############################################################################

add_executable (odbc
    "src/main.cpp")
target_include_directories (odbc
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries (odbc
    PRIVATE
        odb-compiler-lib)

###############################################################################
# Installation
###############################################################################

if (${ODBC_LIB_TYPE} MATCHES "SHARED")
    install (
        TARGETS odbc
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (${INSTALL_DEV})
    install (
        TARGETS odbc
        EXPORT ODBCConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install (
        DIRECTORY "include/odbc"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install (
        DIRECTORY "${PROJECT_BINARY_DIR}/include/odbc"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install (
        EXPORT ODBCConfig
        DESTINATION "share/odbc/cmake")
endif ()

export (
    TARGETS odbc
    NAMESPACE ODBC
    FILE ODBCConfig.cmake)
export (
    PACKAGE ODBC)
