project ("gperf-util"
    LANGUAGES CXX C
    VERSION 3.1.0)

add_library (gperf-util STATIC
    "src/getline.cc"
    "src/getopt1.c"
    "src/getopt.c"
    "src/hash.cc")
target_include_directories (gperf-util
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

