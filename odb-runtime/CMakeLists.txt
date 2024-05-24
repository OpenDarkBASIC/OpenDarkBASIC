project (odb-runtime
    LANGUAGES CXX)

add_library (odb-runtime STATIC
    "src/exit_process.c"
    "src/rpath.c")
