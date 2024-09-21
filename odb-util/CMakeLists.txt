project ("odb-util"
    LANGUAGES C
    VERSION ${ODB_VERSION})

include (CheckCSourceCompiles)
include (CMakeDependentOption)

###############################################################################
# Configuration
###############################################################################

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (DEBUG_FEATURE ON)
endif ()

set (ODBUTIL_LIB_TYPE "SHARED" CACHE STRING "Build either as SHARED or STATIC library")

set (ODBUTIL_BTREE_EXPAND_FACTOR "2" CACHE STRING "When reallocating btree memory, this is the factor with which the buffer grows")
set (ODBUTIL_BTREE_MIN_CAPACITY "32" CACHE STRING "The smallest number of elements to reserve when initializing a btree")
option (ODBUTIL_DEBUG_ASSERT "Enable the ODBUTIL_DEBUG_ASSERT macro" ${DEBUG_FEATURE})
option (ODBUTIL_BTREE_64BIT_KEYS "Enable 64-bit keys for btrees instead of 32-bit keys" OFF)
option (ODBUTIL_BTREE_64BIT_CAPACITY "Enable btrees to allow up to 2^64 entries instead of 2^32" OFF)
option (ODBUTIL_HM_STATS "Track hashmap usage statistics. This will increase
sizeof(struct hm)!" OFF)
option (ODBUTIL_MEM_DEBUGGING "Enable tracking malloc/realloc/free calls to detect memory leaks" ${DEBUG_FEATURE})
cmake_dependent_option (ODBUTIL_MEM_BACKTRACE "Enable generating backtraces to every malloc/realloc call, making it easy to find where memory leaks occur" ON "${ODBUTIL_MEM_DEBUGGING}" OFF)
cmake_dependent_option (ODBUTIL_MEM_HEX_DUMP "Enable printing out hex dumps of the unfreed memory regions" ON "${ODBUTIL_MEM_DEBUGGING}" OFF)
set (ODBUTIL_MEM_BACKTRACE_SIZE "64" CACHE STRING "Sets the maximum stack size (depth) when generating backtraces")
set (ODBUTIL_MEM_HEX_DUMP_SIZE "1024" CACHE STRING "Memory blocks larger than this size will not be dumped")
cmake_dependent_option (ODBUTIL_PROFILING "Enable -pg and -fno-omit-frame-pointer" ON "${ODB_PROFILING}" OFF)
cmake_dependent_option (ODBUTIL_TESTS "Build unit tests for the OpenDarkBASIC SDK" ON "${ODB_TESTS}" OFF)

# Memory debugging uses thread-local storage for its state because we don't want to deal with locks
check_c_source_compiles ("__declspec(thread) int value; int main(void) { return 0; }" MSVC_THREADLOCAL)
check_c_source_compiles ("__thread int value; int main(void) { return 0; }" GCC_THREADLOCAL)
if (MSVC_THREADLOCAL)
    set (ODBUTIL_THREADLOCAL "__declspec(thread)")
endif ()
if (GCC_THREADLOCAL)
    set (ODBUTIL_THREADLOCAL "__thread")
endif ()

# Static assert support in C
check_c_source_compiles ("int main(void) { _Static_assert(1, \"\"); return 0; }" GCC_STATIC_ASSERT)
check_c_source_compiles ("int main(void) { _STATIC_ASSERT(1); return 0; }" MSVC_STATIC_ASSERT)
if (MSVC_STATIC_ASSERT)
    set (ODBUTIL_STATIC_ASSERT "_STATIC_ASSERT(condition)")
elseif (GCC_STATIC_ASSERT)
    set (ODBUTIL_STATIC_ASSERT "_Static_assert(condition, \"(\" #condition \") failed\")")
else ()
    set (ODBUTIL_STATIC_ASSERT "")
endif ()

# For platform variable we use the parent scope ODB_PLATFORM, but it is lower case
string (TOUPPER ${ODB_PLATFORM} ODBUTIL_PLATFORM)

configure_file ("templates/config.h.in" "include/odb-util/config.h")

###############################################################################
# Library definition
###############################################################################

add_library (odb-util ${ODBUTIL_LIB_TYPE}
    "templates/config.h.in"
    "${PROJECT_BINARY_DIR}/include/odb-util/config.h"

    "include/odb-util/backtrace.h"
    "include/odb-util/btree.h"
    "include/odb-util/cli_colors.h"
    "include/odb-util/dynlib.h"
    "include/odb-util/fs.h"
    "include/odb-util/hash.h"
    "include/odb-util/hm.h"
    "include/odb-util/init.h"
    "include/odb-util/log.h"
    "include/odb-util/mem.h"
    "include/odb-util/mfile.h"
    "include/odb-util/mstream.h"
    "include/odb-util/ospath.h"
    "include/odb-util/ospath_list.h"
    "include/odb-util/process.h"
    "include/odb-util/rb.h"
    "include/odb-util/thread.h"
    "include/odb-util/utf8.h"
    "include/odb-util/vec.h"

    "src/btree.c"
    "src/fs_common.c"
    "src/hash.c"
    "src/init.c"
    "src/log.c"
    "src/mstream.c"
    "src/process_common.c"
    "src/rb.c"
    "src/utf8.c"
    "src/utf8_list.c"

    $<$<BOOL:${ODBUTIL_MEM_DEBUGGING}>:src/mem.c>
    
    $<$<PLATFORM_ID:Linux>:$<$<BOOL:${ODBUTIL_MEM_BACKTRACE}>:src/backtrace_linux.c>>
    $<$<PLATFORM_ID:Linux>:src/dynlib_linux.c>
    $<$<PLATFORM_ID:Linux>:src/fs_linux.c>
    $<$<PLATFORM_ID:Linux>:src/mfile_linux.c>
    $<$<PLATFORM_ID:Linux>:src/ospath_linux.c>
    $<$<PLATFORM_ID:Linux>:src/process_linux.c>
    $<$<PLATFORM_ID:Linux>:src/thread_linux.c>
    $<$<PLATFORM_ID:Linux>:src/utf8_linux.c>

    $<$<PLATFORM_ID:Windows>:$<$<BOOL:${ODBUTIL_MEM_BACKTRACE}>:src/backtrace_win32.c>>
    $<$<PLATFORM_ID:Windows>:src/dynlib_win32.c>
    $<$<PLATFORM_ID:Windows>:src/fs_win32.c>
    $<$<PLATFORM_ID:Windows>:src/mfile_win32.c>
    $<$<PLATFORM_ID:Windows>:src/ospath_win32.c>
    $<$<PLATFORM_ID:Windows>:src/process_win32.c>
    $<$<PLATFORM_ID:Windows>:src/thread_win32.c>
    $<$<PLATFORM_ID:Windows>:src/utf8_win32.c>)
target_include_directories (odb-util
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include/odb-util>")
target_compile_definitions (odb-util
    PRIVATE
        ODBUTIL_BUILDING
        _CRT_SECURE_NO_WARNINGS)
target_compile_options (odb-util
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<C_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fvisibility=hidden>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive -fvisibility=hidden>)
include (ODBTargetProperties)
odb_target_properties (odb-util
    PROPERTIES
        MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})

###############################################################################
# Dependencies
###############################################################################

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (odb-util PRIVATE DL::DL)
endif ()

target_link_libraries (odb-util
    PRIVATE
        $<$<PLATFORM_ID:Windows>:$<$<BOOL:${ODBUTIL_MEM_BACKTRACE}>:Dbghelp>>)

include (FetchContent)

if (ODBUTIL_TESTS)
    target_sources (odb-tests PRIVATE
        "tests/src/LogHelper.cpp"
        "tests/src/Utf8Helper.cpp"
        "tests/src/test_odbutil_btree.cpp"
        "tests/src/test_odbutil_btree_as_set.cpp"
        "tests/src/test_odbutil_log.cpp"
        "tests/src/test_odbutil_mem.cpp"
        "tests/src/test_odbutil_hm.cpp"
        "tests/src/test_odbutil_hm_full.cpp"
        "tests/src/test_odbutil_ospath.cpp"
        "tests/src/test_odbutil_process.cpp"
        "tests/src/test_odbutil_rb.cpp"
        "tests/src/test_odbutil_utf8.cpp"
        "tests/src/test_odbutil_utf8_list.cpp"
        "tests/src/test_odbutil_vec.cpp")
    target_include_directories (odb-tests
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests/include>)
    target_link_libraries (odb-tests PUBLIC odb-util)

    add_executable (odb-echo
        "tests/src/echo.c")
    odb_target_properties (odb-echo
        PROPERTIES
            MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>
            RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})
    add_dependencies (odb-tests odb-echo)
endif ()

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odb-util
    EXPORT OpenDarkBASICConfig
    ARCHIVE DESTINATION ${ODB_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${ODB_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})
install (
    DIRECTORY "include/odb-util"
    DESTINATION ${ODB_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-util"
    DESTINATION ${ODB_INSTALL_INCLUDEDIR})
install (
    EXPORT OpenDarkBASICConfig
    DESTINATION "share/odb/cmake")
