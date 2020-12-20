find_program(Gperf_EXECUTABLE NAMES gperf gperf DOC "path to the gperf executable")
mark_as_advanced(Gperf_EXECUTABLE)

if(Gperf_EXECUTABLE)
  execute_process(COMMAND ${Gperf_EXECUTABLE} --version
    OUTPUT_VARIABLE Gperf_version_output
    ERROR_VARIABLE Gperf_version_error
    RESULT_VARIABLE Gperf_version_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT ${Gperf_version_result} EQUAL 0)
    if(Gperf_FIND_REQUIRED)
      message(SEND_ERROR "Command \"${Gperf_EXECUTABLE} --version\" failed with output:\n${Gperf_version_output}\n${Gperf_version_error}")
    else()
      message("Command \"${Gperf_EXECUTABLE} --version\" failed with output:\n${Gperf_version_output}\n${Gperf_version_error}\nGperf_VERSION will not be available")
    endif()
  else()
    get_filename_component(Gperf_EXE_NAME_WE "${Gperf_EXECUTABLE}" NAME_WE)
    get_filename_component(Gperf_EXE_EXT "${Gperf_EXECUTABLE}" EXT)
    string(REGEX REPLACE "^.*${Gperf_EXE_NAME_WE}(${Gperf_EXE_EXT})? ([0-9]+\\.[0-9]+)[^ ]*( .*)?$" "\\2"
      Gperf_VERSION "${Gperf_version_output}")
    unset(Gperf_EXE_EXT)
    unset(Gperf_EXE_NAME_WE)
  endif()

  #============================================================
  # Gperf_TARGET (public macro)
  #============================================================
  #
  macro(Gperf_TARGET Name Input Output)

    set(Gperf_TARGET_PARAM_OPTIONS)
    set(Gperf_TARGET_PARAM_ONE_VALUE_KEYWORDS
      COMPILE_FLAGS
      )
    set(Gperf_TARGET_PARAM_MULTI_VALUE_KEYWORDS)

    cmake_parse_arguments(
      Gperf_TARGET_ARG
      "${Gperf_TARGET_PARAM_OPTIONS}"
      "${Gperf_TARGET_PARAM_ONE_VALUE_KEYWORDS}"
      "${Gperf_TARGET_MULTI_VALUE_KEYWORDS}"
      ${ARGN})

    set(Gperf_TARGET_usage "Gperf_TARGET(<Name> <Input> <Output> [COMPILE_FLAGS <string>]")

    if(NOT "${Gperf_TARGET_ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
      message(SEND_ERROR ${Gperf_TARGET_usage})
    else()

      cmake_policy(GET CMP0098 _gperf_CMP0098
          PARENT_SCOPE # undocumented, do not use outside of CMake
        )
      set(_gperf_INPUT "${Input}")
      if("x${_gperf_CMP0098}x" STREQUAL "xNEWx")
        set(_gperf_WORKING_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        if(NOT IS_ABSOLUTE "${_gperf_INPUT}")
          set(_gperf_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_gperf_INPUT}")
        endif()
      else()
        set(_gperf_WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
      endif()
      unset(_gperf_CMP0098)

      set(_gperf_OUTPUT "${Output}")
      if(NOT IS_ABSOLUTE ${_gperf_OUTPUT})
        set(_gperf_OUTPUT "${_gperf_WORKING_DIR}/${_gperf_OUTPUT}")
      endif()
      set(_gperf_TARGET_OUTPUTS "${_gperf_OUTPUT}")

      set(_gperf_EXE_OPTS "")
      if(NOT "${Gperf_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
        set(_gperf_EXE_OPTS "${Gperf_TARGET_ARG_COMPILE_FLAGS}")
        separate_arguments(_gperf_EXE_OPTS)
      endif()

      set(_gperf_OUTPUT_HEADER "")
      if(NOT "${Gperf_TARGET_ARG_DEFINES_FILE}" STREQUAL "")
        set(_gperf_OUTPUT_HEADER "${Gperf_TARGET_ARG_DEFINES_FILE}")
        if(IS_ABSOLUTE "${_gperf_OUTPUT_HEADER}")
          set(_gperf_OUTPUT_HEADER_ABS "${_gperf_OUTPUT_HEADER}")
        else()
          set(_gperf_OUTPUT_HEADER_ABS "${_gperf_WORKING_DIR}/${_gperf_OUTPUT_HEADER}")
        endif()
        list(APPEND _gperf_TARGET_OUTPUTS "${_gperf_OUTPUT_HEADER_ABS}")
        list(APPEND _gperf_EXE_OPTS --header-file=${_gperf_OUTPUT_HEADER_ABS})
      endif()

      get_filename_component(_gperf_EXE_NAME_WE "${Gperf_EXECUTABLE}" NAME_WE)
      add_custom_command(OUTPUT ${_gperf_TARGET_OUTPUTS}
        COMMAND ${Gperf_EXECUTABLE} ${_gperf_EXE_OPTS} --output-file=${_gperf_OUTPUT} ${_gperf_INPUT}
        VERBATIM
        DEPENDS ${_gperf_INPUT}
        COMMENT "[Gperf][${Name}] Calculating perfect hash table with ${_gperf_EXE_NAME_WE} ${Gperf_VERSION}"
        WORKING_DIRECTORY ${_gperf_WORKING_DIR})

      set(Gperf_${Name}_DEFINED TRUE)
      set(Gperf_${Name}_OUTPUTS ${_gperf_TARGET_OUTPUTS})
      set(Gperf_${Name}_INPUT ${_gperf_INPUT})
      set(Gperf_${Name}_COMPILE_FLAGS ${_gperf_EXE_OPTS})
      set(Gperf_${Name}_OUTPUT_HEADER ${_gperf_OUTPUT_HEADER})

      unset(_gperf_EXE_NAME_WE)
      unset(_gperf_EXE_OPTS)
      unset(_gperf_INPUT)
      unset(_gperf_OUTPUT)
      unset(_gperf_OUTPUT_HEADER)
      unset(_gperf_OUTPUT_HEADER_ABS)
      unset(_gperf_TARGET_OUTPUTS)
      unset(_gperf_WORKING_DIR)

    endif()
  endmacro()

  macro(ADD_GPERF_BISON_DEPENDENCY GperfTarget BisonTarget)

    if(NOT Gperf_${GperfTarget}_OUTPUTS)
      message(SEND_ERROR "Gperf target `${GperfTarget}' does not exist.")
    endif()

    if(NOT BISON_${BisonTarget}_OUTPUT_HEADER)
      message(SEND_ERROR "Bison target `${BisonTarget}' does not exist.")
    endif()

    set_source_files_properties(${Gperf_${GperfTarget}_OUTPUTS}
      PROPERTIES OBJECT_DEPENDS ${BISON_${BisonTarget}_OUTPUT_HEADER})
  endmacro()

endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gperf REQUIRED_VARS Gperf_EXECUTABLE
                                       VERSION_VAR Gperf_VERSION)
