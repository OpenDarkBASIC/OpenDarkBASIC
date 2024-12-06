include (GNUInstallDirs)

project ("odb-cli"
    LANGUAGES C
    VERSION 0.0.1)

###############################################################################
# Executable
###############################################################################

set (odb-cli_DEPGEN_OUTPUT "${PROJECT_BINARY_DIR}/include/odb-cli/args.cli.h")
odb_cligen_target (odb-cli_depgen
    INPUT "${PROJECT_SOURCE_DIR}/src/args.cli"
    OUTPUT "${odb-cli_DEPGEN_OUTPUT}")

add_executable (odb-cli
    ${odb-cli_DEPGEN_OUTPUT}
    "src/main.c")
target_include_directories (odb-cli
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries (odb-cli
    PRIVATE
        odb-util
        odb-compiler)
target_compile_definitions (odb-cli
    PRIVATE 
        $<$<CXX_COMPILER_ID:MSVC>:_HAS_EXCEPTIONS=0>)
include (ODBTargetProperties)
odb_target_properties (odb-cli
    PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR}
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
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
