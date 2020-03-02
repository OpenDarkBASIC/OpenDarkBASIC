project ("test-plugin"
    VERSION 0.0.1)

add_library (test-plugin SHARED
    "src/print.c")
target_include_directories (test-plugin
    PRIVATE
        "include")
set_target_properties (test-plugin
    PROPERTIES
        PREFIX "")
