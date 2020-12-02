list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
set (PLUGIN_CONFIG_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/templates")

add_subdirectory ("test-plugin")
add_subdirectory ("core-commands")

