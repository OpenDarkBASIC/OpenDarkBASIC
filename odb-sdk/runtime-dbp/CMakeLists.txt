list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

option (ODBSDK_BUILD_DBP_RUNTIME "Build DBP ODB runtime DLL." OFF)
set (ODBSDK_DBP_RUNTIME_GENERATOR "" CACHE STRING "CMake Visual Studio code generator to use for building the DBP odb runtime DLL. Defaults to the current code generator if it is a Visual Studio generator.")

if (ODBSDK_BUILD_DBP_RUNTIME)
    if (NOT WIN32)
        message (FATAL_ERROR "ODBSDK_BUILD_DBP_RUNTIME is only supported on Windows.")
    endif()

    # We need to cross compile the DBP runtime to x86. Do that in an ExternalProject.
    include (ExternalProject)
    if (ODBSDK_DBP_RUNTIME_GENERATOR STREQUAL "")
        if (CMAKE_GENERATOR MATCHES "Visual Studio")
            set(ODB_RUNTIME_GENERATOR ${CMAKE_GENERATOR})
        else ()
            # If you use a different generator, it needs to be specified explicitly, as we won't be able to figure out
            # the right VS generator to use.
            message (FATAL_ERROR "The ODBSDK_DBP_RUNTIME_GENERATOR option must be specified if you're not using a Visual Studio generator.")
        endif ()
    else ()
        set(ODB_RUNTIME_GENERATOR ${ODBSDK_DBP_RUNTIME_GENERATOR})
    endif ()
    ExternalProject_Add (odb-runtime-dbp
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/runtime
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${ODB_DBP_SDK_DIR}
        CMAKE_GENERATOR ${ODB_RUNTIME_GENERATOR}
        CMAKE_GENERATOR_PLATFORM "Win32"
        # This is required as CMake can't detect if anything in the runtime folder has been changed due to being an
        # external project.
        BUILD_ALWAYS 1
    )
else ()
    # TODO: We need to find a pre-built odb-runtime-dbp.dll.
endif ()
