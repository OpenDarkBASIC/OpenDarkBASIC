project (odb-runtime
    LANGUAGES C)

configure_file ("templates/config.h.in" "include/odb-runtime/config.h")

add_library (odb-runtime SHARED
    "src/odbrt.c")
target_include_directories (odb-runtime
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries (odb-runtime
    PRIVATE
        odb-util)
include (ODBTargetProperties)
odb_target_properties (odb-runtime
    ARCHIVE_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/runtime"
    LIBRARY_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/runtime"
    RUNTIME_OUTPUT_DIRECTORY "${ODB_BUILD_SDKDIR}/runtime")

###############################################################################
# Windows SDK
###############################################################################

set (NEED_CRT OFF)
if (WIN32 AND NEED_CRT)
    if (NOT WINDOWS_KIT_DIR)
        get_filename_component (WINDOWS_KITS_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v10.0;InstallationFolder]" ABSOLUTE CACHE)
        if ("${WINDOWS_KITS_DIR}" STREQUAL "/registry")
            message (FATAL_ERROR "Unable to find installed Windows SDK")
        endif()

        file (GLOB WINDOWS_KITS_VERSIONS RELATIVE "${WINDOWS_KITS_DIR}/Lib" "${WINDOWS_KITS_DIR}/Lib/*")
        list (FILTER WINDOWS_KITS_VERSIONS INCLUDE REGEX "10\\.0\\.")
        list (SORT WINDOWS_KITS_VERSIONS COMPARE NATURAL ORDER DESCENDING)
        list (POP_FRONT WINDOWS_KITS_VERSIONS WINDOWS_KIT_VERSION)
        set (WINDOWS_KIT_DIR "${WINDOWS_KITS_DIR}/Lib/${WINDOWS_KIT_VERSION}" CACHE STRING "Path to the Windows Kit")
    endif ()
    
    if (NOT MSVC_DIR)
        execute_process (
            COMMAND cmd /C "where link"
            OUTPUT_VARIABLE MSVC_DIR)
        get_filename_component (MSVC_DIR "${MSVC_DIR}" DIRECTORY)
        get_filename_component (MSVC_DIR "${MSVC_DIR}" DIRECTORY)
        get_filename_component (MSVC_DIR "${MSVC_DIR}" DIRECTORY)
        get_filename_component (MSVC_DIR "${MSVC_DIR}" DIRECTORY)
        set (MSVC_DIR ${MSVC_DIR} CACHE STRING "Path to MSVC")
    endif ()
    
    if (ODB_ARCH MATCHES "i386")
        set (ARCH "x86")
    elseif (ODB_ARCH MATCHES "x86_64")
        set (ARCH "x64")
    else ()
        message (FATAL_ERROR "Failed to map ODB_ARCH to a Windows arch")
    endif ()

    # These files are required for ODB compiler to link executables correctly on Windows
    add_custom_command (TARGET odb-runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ODB_BUILD_SDKDIR}/runtime"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MSVC_DIR}/lib/onecore/${ARCH}/libvcruntime.lib" "${ODB_BUILD_SDKDIR}/runtime/libvcruntime.lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MSVC_DIR}/lib/onecore/${ARCH}/libcmt.lib" "${ODB_BUILD_SDKDIR}/runtime/libcmt.lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WINDOWS_KIT_DIR}/ucrt/${ARCH}/libucrt.lib" "${ODB_BUILD_SDKDIR}/runtime/libucrt.lib")
endif()
