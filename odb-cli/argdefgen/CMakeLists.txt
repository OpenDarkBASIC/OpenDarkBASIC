project (argdefgen
    LANGUAGES C
    VERSION 1.0.0)

find_package (FLEX REQUIRED 2.6)
find_package (BISON REQUIRED 3.2)

# These may not exist
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src")
file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/argdefgen")

bison_target (adg_parser
    "${PROJECT_SOURCE_DIR}/src/parser.y"
    "${PROJECT_BINARY_DIR}/src/parser.y.c"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/argdefgen/parser.y.h"
    COMPILE_FLAGS -Wcounterexamples)
flex_target (adg_scanner
    "${PROJECT_SOURCE_DIR}/src/scanner.lex"
    "${PROJECT_BINARY_DIR}/src/scanner.lex.c"
    DEFINES_FILE "${PROJECT_BINARY_DIR}/include/argdefgen/scanner.lex.h")
add_flex_bison_dependency (adg_scanner adg_parser)

add_executable (argdefgen
    "src/action.c"
    "src/driver.c"
    "src/gen_cpp.c"
    "src/main.c"
    "src/node.c"
    "src/str.c"
    ${BISON_adg_parser_OUTPUTS}
    ${FLEX_adg_scanner_OUTPUTS})
target_include_directories (argdefgen
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
