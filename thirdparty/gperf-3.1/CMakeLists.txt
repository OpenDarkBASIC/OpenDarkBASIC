cmake_minimum_required (VERSION 3.21)

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

project ("GNU gperf")

add_subdirectory ("gperf-util")
add_subdirectory ("gperf")

macro (Gperf_TARGET Name Input Output)
	set (Gperf_TARGET_PARAM_OPTIONS)
	set (Gperf_TARGET_PARAM_ONE_VALUE_KEYWORDS
		COMPILE_FLAGS)
	set (Gperf_TARGET_PARAM_MULTI_VALUE_KEYWORDS)
	
	cmake_parse_arguments (
		Gperf_TARGET_ARG
		"${Gperf_TARGET_PARAM_OPTIONS}"
		"${Gperf_TARGET_PARAM_ONE_VALUE_KEYWORDS}"
		"${Gperf_TARGET_PARAM_MULTI_VALUE_KEYWORDS}"
		${ARGN})
	
	if (NOT "${Gperf_TARGET_ARG_UNPARSED_ARGUMENTS}" STREQUAL "")
		message (FATAL_ERROR "gperf_target(<Name> <Input> <Output> [COMPILE_FLAGS <string>]")
	endif ()
	
	set (_gperf_INPUT "${Input}")
	set (_gperf_WORKING_DIR "${CMAKE_CURRENT_BINARY_DIR}")
	if (NOT IS_ABSOLUTE "${_gperf_INPUT}")
		set (_gperf_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${_gperf_INPUT}")
	endif ()
	
	set (_gperf_OUTPUT "${Output}")
	if (NOT IS_ABSOLUTE ${_gperf_OUTPUT})
		set (_gperf_OUTPUT "${_gperf_WORKING_DIR}/${_gperf_OUTPUT}")
	endif ()
	
	set (_gperf_EXE_OPTS "")
	if (NOT "${Gperf_TARGET_ARG_COMPILE_FLAGS}" STREQUAL "")
		set (_gperf_EXE_OPTS  "${Gperf_TARGET_ARG_COMPILE_FLAGS}")
		separate_arguments (_gperf_EXE_OPTS)
	endif ()
	
	add_custom_command (OUTPUT ${_gperf_OUTPUT}
		COMMAND gperf ${_gperf_EXE_OPTS} --output-file=${_gperf_OUTPUT} ${_gperf_INPUT}
		VERBATIM
		DEPENDS ${_gperf_INPUT}
		COMMENT "[Gperf][${Name}] Calculating perfect hash table"
		WORKING_DIRECTORY ${_gperf_WORKING_DIR})
	
	set (Gperf_${Name}_DEFINED TRUE)
	set (Gperf_${Name}_OUTPUTS ${_gperf_OUTPUT})
	set (Gperf_${Name}_INPUT ${_gperf_INPUT})
	set (Gperf_${NAME}_COMPILE_FLAGS ${_gperf_EXE_OPTS})
		
	unset (_gperf_EXE_OPTS)
	unset (_gperf_INPUT)
	unset (_gperf_OUTPUT)
	unset (_gperf_WORKING_DIR)
endmacro ()

macro (add_gperf_bison_dependency GperfTarget BisonTarget)
	if(NOT Gperf_${GperfTarget}_OUTPUTS)
		message(FATAL_ERROR "Gperf target `${GperfTarget}' does not exist.")
    endif()

    if(NOT BISON_${BisonTarget}_OUTPUT_HEADER)
		message(FATAL_ERROR "Bison target `${BisonTarget}' does not exist.")
    endif()

    set_source_files_properties(${Gperf_${GperfTarget}_OUTPUTS}
		PROPERTIES
			OBJECT_DEPENDS ${BISON_${BisonTarget}_OUTPUT_HEADER})
endmacro ()
