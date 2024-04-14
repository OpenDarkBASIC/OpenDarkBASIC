include (ODBPlugin)

project ("test-plugin"
    VERSION 0.0.1
    LANGUAGES C)

odb_add_plugin (test-plugin
    SOURCES
        "src/test_command.c"
        "$<$<PLATFORM_ID:Windows>:src/strings.rc>"
    INCLUDE_DIRECTORIES
        "include")
