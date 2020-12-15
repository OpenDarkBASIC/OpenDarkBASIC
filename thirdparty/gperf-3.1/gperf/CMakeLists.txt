include (CheckCXXSourceCompiles)

project ("gperf"
    LANGUAGES C CXX
    VERSION 3.1.0)

check_cxx_source_compiles ("
int main(int argc, char** argv) {
    int arr[argc];
}" HAVE_DYNAMIC_ARRAY)
set (GPERF_PACKAGE_BUGREPORT "" CACHE STRING "Define to the address where bug reports for this package should be sent.")
set (GPERF_PACKAGE_NAME "" CACHE STRING "Define to the full name of this package.")
set (GPERF_PACKAGE_STRING "" CACHE STRING "Define to the full name and version of this package.")
set (GPERF_PACKAGE_TARNAME "" CACHE STRING "Define to the one symbol short name of this package.")
set (GPERF_PACKAGE_URL "" CACHE STRING "Define to the home page for this package.")
set (GPERF_PACKAGE_VERSION "" CACHE STRING "Define to the version of this package.")

set (PACKAGE_BUGREPORT ${GPERF_PACKAGE_BUGREPORT})
set (PACKAGE_NAME ${GPERF_PACKAGE_NAME})
set (PACKAGE_STRING ${GPERF_PACKAGE_STRING})
set (PACKAGE_TARNAME ${GPERF_PACKAGE_TARNAME})
set (PACKAGE_URL ${GPERF_PACKAGE_URL})
set (PACKAGE_VERSION ${GPERF_PACKAGE_VERSION})

configure_file ("templates/config.h.in" "include/gperf/config.h")
configure_file ("templates/version.cc.in" "src/version.cc")

add_executable (gperf
    "src/bool-array.cc"
    "src/hash-table.cc"
    "src/input.cc"
    "src/keyword.cc"
    "src/keyword-list.cc"
    "src/options.cc"
    "src/output.cc"
    "src/positions.cc"
    "src/search.cc"
    "src/main.cc"
    "${PROJECT_BINARY_DIR}/src/version.cc")
target_include_directories (gperf
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries (gperf
    PRIVATE
        gperf-util)

