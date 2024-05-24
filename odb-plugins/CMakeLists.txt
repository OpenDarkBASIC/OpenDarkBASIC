list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
set (PLUGIN_CONFIG_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/templates")

include (TestVisibilityMacros)
test_visibility_macros (
    ODBPLUGIN_API_EXPORT
    ODBPLUGIN_API_IMPORT
    ODBPLUGIN_API_LOCAL)

add_subdirectory ("test-plugin")
add_subdirectory ("core-commands")
