list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

add_library (odb-runtime SHARED "src/Runtime.cpp")
