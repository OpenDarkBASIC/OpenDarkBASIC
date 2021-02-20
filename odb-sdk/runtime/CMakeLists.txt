list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

if (WIN32)
    add_library (odb-runtime-dbp SHARED "src/RuntimeDBP.cpp")
else ()
    message (STATUS "odb-runtime-dbp.dll needs to be provided in binary format to support DBP SDK")
endif ()

add_library (odb-runtime SHARED "src/Runtime.cpp")