macro (odb_add_plugin PLUGIN)
    set (_options "")
    set (_oneValueArgs "")
    set (_multiValueArgs SOURCES HEADERS INCLUDE_DIRECTORIES)
    cmake_parse_arguments (${PLUGIN} "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

    configure_file (
      "${PLUGIN_CONFIG_TEMPLATE_PATH}/config.h.in"
      "${PROJECT_BINARY_DIR}/include/${PLUGIN}/config.h")

    set (RESGEN_TARGET "elf")
    if (WIN32 OR CYGWIN)
        set (RESGEN_TARGET "winres")
    endif ()
    odb_resgen_target (${PLUGIN}
        TARGET ${RESGEN_TARGET}
        INPUT
            ${${PLUGIN}_SOURCES}
            ${${PLUGIN}_HEADERS})

    add_library (${PLUGIN} SHARED
        ${${PLUGIN}_SOURCES}
        ${${PLUGIN}_HEADERS}
        ${ODB_RESGEN_${PLUGIN}_OUTPUTS})
    target_include_directories (${PLUGIN}
        PRIVATE
            ${${PLUGIN}_INCLUDE_DIRECTORIES}
            "${PROJECT_BINARY_DIR}/include")
    target_compile_options (${PLUGIN}
        PRIVATE
            $<$<C_COMPILER_ID:GNU>:-fvisibility=hidden>
            $<$<CXX_COMPILER_ID:GNU>:-fvisibility=hidden>
            $<$<C_COMPILER_ID:Clang>:-fvisibility=hidden>
            $<$<CXX_COMPILER_ID:Clang>:-fvisibility=hidden>
            $<$<C_COMPILER_ID:AppleClang>:-fvisibility=hidden>
            $<$<CXX_COMPILER_ID:AppleClang>:-fvisibility=hidden>)
    target_link_libraries (${PLUGIN}
        PRIVATE
            odb-sdk)
    include (ODBTargetProperties)
    odb_target_properties (${PLUGIN}
        PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/plugins"
            RUNTIME_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/plugins")
    set_property (TARGET ${PLUGIN}
        PROPERTY PREFIX "")

    install (
        TARGETS ${PLUGIN}
        LIBRARY DESTINATION "${ODB_INSTALL_SDKDIR}/plugins"
        RUNTIME DESTINATION "${ODB_INSTALL_SDKDIR}/plugins")
    
    unset (_options)
    unset (_oneValueArgs)
    unset (_multiValueArgs)
endmacro ()
