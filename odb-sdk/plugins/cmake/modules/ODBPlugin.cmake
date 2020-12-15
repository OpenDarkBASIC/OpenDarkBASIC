macro (odb_add_plugin PLUGIN)
    set (options "")
    set (oneValueArgs "")
    set (multiValueArgs SOURCES HEADERS INCLUDE_DIRECTORIES)
    cmake_parse_arguments (${PLUGIN} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

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
    set_target_properties (${PLUGIN}
        PROPERTIES
            PREFIX "")
    set_target_properties (${PLUGIN}
        PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${ODB_SDK_DIR}/plugins"
            RUNTIME_OUTPUT_DIRECTORY "${ODB_SDK_DIR}/plugins")
    install (
        TARGETS ${PLUGIN}
        LIBRARY DESTINATION "${CMAKE_INSTALL_ODBSDKDIR}/plugins"
        RUNTIME DESTINATION "${CMAKE_INSTALL_ODBSDKDIR}/plugins")
endmacro ()
