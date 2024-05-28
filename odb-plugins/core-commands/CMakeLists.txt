include (ODBPlugin)

project ("core-commands"
    VERSION 0.0.1
    LANGUAGES CXX)

odb_add_plugin (core-commands
    SOURCES
        "src/dll.c"
        "src/print_stdout.c"
        "src/str.c"
        "src/val.c"
    INCLUDE_DIRECTORIES
        "include")
