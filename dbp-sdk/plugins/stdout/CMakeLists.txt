cmake_minimum_required (VERSION 3.21)

project (dbp-stdout
    LANGUAGES C)

add_library (dbp-stdout SHARED
    "src/stdout.c")
