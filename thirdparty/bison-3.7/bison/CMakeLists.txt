cmake_minimum_required (VERSION 3.3)

project ("bison"
    LANGUAGES C
    VERSION 3.7.0)

add_executable (bison
    "src/AnnotationList.c"
    "src/assoc.c"
    "src/closure.c"
    "src/complain.c"
    "src/conflicts.c"
    "src/counterexample.c"
    "src/derivation.c"
    "src/derives.c"
    "src/files.c"
    "src/fixits.c"
    "src/getargs.c"
    "src/glyphs.c"
    "src/gram.c"
    "src/graphviz.c"
    #"src/i18n-strings.c"
    "src/ielr.c"
    "src/InadequacyList.c"
    "src/lalr.c"
    "src/location.c"
    "src/lr0.c"
    "src/lssi.c"
    "src/muscle-tab.c"
    "src/named-ref.c"
    "src/nullable.c"
    "src/output.c"
    "src/parse-gram.c"
    "src/parse-simulation.c"
    "src/print.c"
    "src/print-graph.c"
    "src/print-xml.c"
    "src/reader.c"
    "src/reduce.c"
    "src/relation.c"
    "src/Sbitset.c"
    "src/scan-code-c.c"
    "src/scan-gram-c.c"
    "src/scan-skel-c.c"
    "src/state.c"
    "src/state-item.c"
    "src/symlist.c"
    "src/symtab.c"
    "src/tables.c"
    "src/uniqstr.c"
    "src/main.c")

target_include_directories (bison
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries (bison
    PRIVATE
        bison-util)
