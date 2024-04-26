include (GNUInstallDirs)

project ("odb-cli"
    LANGUAGES CXX
    VERSION 0.0.1)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-cli")

add_subdirectory ("argdefgen")
argdefgen_target (odb-cli_argdef
    "${PROJECT_SOURCE_DIR}/src/Actions.argdef"
    "${PROJECT_BINARY_DIR}/src/Actions.argdef.cpp"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/odb-cli/Actions.argdef.hpp")

###############################################################################
# Executable
###############################################################################

add_executable (odb-cli
    ${argdefgen_odb-cli_argdef_OUTPUTS}
    #"src/AST.cpp"
    "src/Banner.cpp"
    "src/BuildInfo.cpp"
    "src/Codegen.cpp"
    #"src/Commands.cpp"
    "src/Log.cpp"
    #"src/SDK.cpp"
    #"src/Warnings.cpp"
    "src/main.cpp")
target_include_directories (odb-cli
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries (odb-cli
    PRIVATE
        odb-sdk
        odb-compiler)
include (ODBTargetProperties)
odb_target_properties (odb-cli
    PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR}
        INSTALL_RPATH ${ODB_INSTALL_LIBDIR})

# Set up a job to copy LLD binaries into odb-cli's bin folder.
#add_custom_command (TARGET odb-cli
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${ODB_BUILD_BINDIR}/lld/bin
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${ODB_BUILD_BINDIR}/lld/lib
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${LLVM_BINARY_DIR}/bin/lld${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${LLVM_BINARY_DIR}/bin/lld-link${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${LLVM_BINARY_DIR}/bin/ld.lld${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${LLVM_BINARY_DIR}/bin/ld64.lld${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin
#    BYPRODUCTS
#        ${ODB_BUILD_BINDIR}/lld/bin/lld${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin/lld-link${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin/ld.lld${CMAKE_EXECUTABLE_SUFFIX}
#        ${ODB_BUILD_BINDIR}/lld/bin/ld64.lld${CMAKE_EXECUTABLE_SUFFIX})

#if (NOT MSVC)
#    add_custom_command (TARGET odb-cli
#        COMMAND ${CMAKE_COMMAND} -E copy
#            ${LLVM_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}LLVM${CMAKE_SHARED_LIBRARY_SUFFIX}
#            ${ODB_BUILD_BINDIR}/lld/lib
#        BYPRODUCTS
#            ${ODB_BUILD_BINDIR}/lld/lib/${CMAKE_SHARED_LIBRARY_PREFIX}LLVM${CMAKE_SHARED_LIBRARY_SUFFIX}
#    )
#endif ()

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odb-cli
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})
#install (
#    DIRECTORY ${ODB_BUILD_BINDIR}/lld
#    DESTINATION ${ODB_INSTALL_BINDIR})
