macro (odb_add_plugin PLUGIN)
    set (_options "")
    set (_oneValueArgs "")
    set (_multiValueArgs SOURCES HEADERS INCLUDE_DIRECTORIES)
    cmake_parse_arguments (${PLUGIN} "${_options}" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

    configure_file ("${PLUGIN_CONFIG_TEMPLATE_PATH}/config.hpp.in"
                    "${PROJECT_BINARY_DIR}/include/${PLUGIN}/config.hpp")

    add_library (${PLUGIN} SHARED
        ${${PLUGIN}_SOURCES}
        ${${PLUGIN}_HEADERS})
    target_include_directories (${PLUGIN}
        PRIVATE
            ${${PLUGIN}_INCLUDE_DIRECTORIES}
            "${PROJECT_BINARY_DIR}/include")
    target_compile_definitions (${PLUGIN}
        PRIVATE
            ODBPLUGIN_BUILDING)
    target_link_libraries (${PLUGIN}
        PRIVATE
            odb-sdk)
    include (ODBTargetProperties)
    set_target_properties (${PLUGIN}
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED TRUE
            LIBRARY_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/plugins"
            RUNTIME_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/plugins"
            MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
    set_property (TARGET ${PLUGIN} PROPERTY PREFIX "")
    install (
        TARGETS ${PLUGIN}
        LIBRARY DESTINATION "${ODB_INSTALL_SDKDIR}/plugins"
        RUNTIME DESTINATION "${ODB_INSTALL_SDKDIR}/plugins")
    
    unset (_options)
    unset (_oneValueArgs)
    unset (_multiValueArgs)
endmacro ()
