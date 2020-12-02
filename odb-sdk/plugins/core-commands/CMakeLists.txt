include (ODBPlugin)

project ("core-commands"
    VERSION 0.0.1
    LANGUAGES CXX)

odb_add_plugin (core-commands
    SOURCES
        "src/str.cpp"
    INCLUDE_DIRECTORIES
        "include")

