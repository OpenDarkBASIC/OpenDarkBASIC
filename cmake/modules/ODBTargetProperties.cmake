macro (odb_target_properties TARGET)
    cmake_parse_arguments (${TARGET}
        "PROPERTIES"
        "RUNTIME_OUTPUT_DIRECTORY;LIBRARY_OUTPUT_DIRECTORY;ARCHIVE_OUTPUT_DIRECTORY"
        ""
        ${ARGN})
    
    set (_types RUNTIME LIBRARY ARCHIVE)
    
    foreach (_type ${_types})
        if (${TARGET}_${_type}_OUTPUT_DIRECTORY)
            foreach (_config ${CMAKE_CONFIGURATION_TYPES})
                string (TOUPPER ${_config} _CONFIG)
                set_target_properties (${TARGET} PROPERTIES
                    ${_type}_OUTPUT_DIRECTORY_${_CONFIG} "${${TARGET}_${_type}_OUTPUT_DIRECTORY}")
            endforeach ()
            if (NOT CMAKE_CONFIGURATION_TYPES)
                set_target_properties (${TARGET} PROPERTIES
                    ${_type}_OUTPUT_DIRECTORY "${${TARGET}_${_type}_OUTPUT_DIRECTORY}")
            endif ()
        endif ()
    endforeach ()
    
    set_target_properties (${TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE)
    
    if (${TARGET}_UNPARSED_ARGUMENTS)
        set_target_properties (${TARGET} PROPERTIES
            ${${TARGET}_UNPARSED_ARGUMENTS})
    endif ()
    
    unset (_type)
    unset (_types)
    unset (_CONFIG)
    unset (_config)
endmacro ()
