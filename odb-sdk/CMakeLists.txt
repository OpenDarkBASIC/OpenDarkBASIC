project ("odb-sdk"
    LANGUAGES CXX C
    VERSION ${ODB_VERSION})

include (CheckCSourceCompiles)
include (CMakeDependentOption)
include (TestVisibilityMacros)

###############################################################################
# Configuration
###############################################################################

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (DEBUG_FEATURE ON)
endif ()

set (ODBSDK_LIB_TYPE "SHARED" CACHE STRING "Build either as SHARED or STATIC library")

set (ODBSDK_BTREE_EXPAND_FACTOR "2" CACHE STRING "When reallocating btree memory, this is the factor with which the buffer grows")
set (ODBSDK_BTREE_MIN_CAPACITY "32" CACHE STRING "The smallest number of elements to reserve when initializing a btree")
option (ODBSDK_BTREE_64BIT_KEYS "Enable 64-bit keys for btrees instead of 32-bit keys" OFF)
option (ODBSDK_BTREE_64BIT_CAPACITY "Enable btrees to allow up to 2^64 entries instead of 2^32" OFF)
option (ODBSDK_HM_STATS "Track hashmap usage statistics. This will increase
sizeof(struct hm)!" OFF)
set (ODBSDK_HM_REHASH_AT_PERCENT "70" CACHE STRING "How full the hash table needs to be before triggering a rehash, in percent")
set (ODBSDK_HM_MIN_CAPACITY "128" CACHE STRING "Default table size when creating new hashmaps")
set (ODBSDK_HM_EXPAND_FACTOR "4" CACHE STRING "When reallocating hashmap memory, thiis is the factor with which the buffer grows. WARNING: MUST be a power of 2!")
option (ODBSDK_MEM_DEBUGGING "Enable tracking malloc/realloc/free calls to detect memory leaks" ${DEBUG_FEATURE})
cmake_dependent_option (ODBSDK_MEM_BACKTRACE "Enable generating backtraces to every malloc/realloc call, making it easy to find where memory leaks occur" ON "${ODBSDK_MEM_DEBUGGING}" OFF)
cmake_dependent_option (ODBSDK_MEM_HEX_DUMP "Enable printing out hex dumps of the unfreed memory regions" ON "${ODBSDK_MEM_DEBUGGING}" OFF)
set (ODBSDK_MEM_BACKTRACE_SIZE "64" CACHE STRING "Sets the maximum stack size (depth) when generating backtraces")
set (ODBSDK_MEM_HEX_DUMP_SIZE "1024" CACHE STRING "Memory blocks larger than this size will not be dumped")
cmake_dependent_option (ODBSDK_PROFILING "Enable -pg and -fno-omit-frame-pointer" ON "${ODB_PROFILING}" OFF)
cmake_dependent_option (ODBSDK_TESTS "Build unit tests for the OpenDarkBASIC SDK" ON "${ODB_TESTS}" OFF)
set (ODBSDK_VEC_EXPAND_FACTOR "2" CACHE STRING "When reallocating vector memory, this is the factor with which the buffer grows")
set (ODBSDK_VEC_MIN_CAPACITY "32" CACHE STRING "The smallest number of elements to reserve when initializing a vector")

string (TOUPPER ${CMAKE_SYSTEM_NAME} ODBSDK_PLATFORM)

test_visibility_macros (
    ODBSDK_API_EXPORT
    ODBSDK_API_IMPORT
    ODBSDK_API_LOCAL)

# Memory debugging uses thread-local storage for its state because we don't want to deal with locks
check_c_source_compiles ("__declspec(thread) int value; int main(void) { return 0; }" MSVC_THREADLOCAL)
check_c_source_compiles ("__thread int value; int main(void) { return 0; }" GCC_THREADLOCAL)
if (MSVC_THREADLOCAL)
    set (ODBSDK_THREADLOCAL "__declspec(thread)")
endif ()
if (GCC_THREADLOCAL)
    set (ODBSDK_THREADLOCAL "__thread")
endif ()

# Static assert support in C
check_c_source_compiles ("int main(void) { _Static_assert(1, \"\"); return 0; }" GCC_STATIC_ASSERT)
check_c_source_compiles ("int main(void) { _STATIC_ASSERT(1); return 0; }" MSVC_STATIC_ASSERT)
if (MSVC_STATIC_ASSERT)
    set (ODBSDK_STATIC_ASSERT "_STATIC_ASSERT(condition)")
elseif (GCC_STATIC_ASSERT)
    set (ODBSDK_STATIC_ASSERT "_Static_assert(condition, \"(\" #condition \") failed\")")
else ()
    set (ODBSDK_STATIC_ASSERT "assert(condition)")
endif ()

configure_file ("templates/config.h.in" "include/odb-sdk/config.h")

###############################################################################
# Library definition
###############################################################################

add_library (odb-sdk ${ODBSDK_LIB_TYPE}
    "templates/config.h.in"
    "${PROJECT_BINARY_DIR}/include/odb-sdk/config.h"

    "include/odb-sdk/backtrace.h"
    "include/odb-sdk/btree.h"
    "include/odb-sdk/cli_colors.h"
    "include/odb-sdk/dynlib.h"
    "include/odb-sdk/fs.h"
    "include/odb-sdk/hash.h"
    "include/odb-sdk/hm.h"
    "include/odb-sdk/init.h"
    "include/odb-sdk/log.h"
    "include/odb-sdk/mem.h"
    "include/odb-sdk/mfile.h"
    "include/odb-sdk/mstream.h"
    "include/odb-sdk/ospath.h"
    "include/odb-sdk/rb.h"
    "include/odb-sdk/thread.h"
    "include/odb-sdk/utf8.h"
    "include/odb-sdk/vec.h"

    "src/btree.c"
    "src/fs_common.c"
    "src/hash.c"
    "src/hm.c"
    "src/init.c"
    "src/log.c"
    "src/mstream.c"
    "src/rb.c"
    "src/utf8.c"
    "src/utf8_list.c"
    "src/vec.c"

    $<$<BOOL:${ODBSDK_MEM_DEBUGGING}>:src/mem.c>

    $<$<PLATFORM_ID:Linux>:src/backtrace_linux.c>
    $<$<PLATFORM_ID:Linux>:src/dynlib_linux.c>
    $<$<PLATFORM_ID:Linux>:src/fs_linux.c>
    $<$<PLATFORM_ID:Linux>:src/mfile_linux.c>
    $<$<PLATFORM_ID:Linux>:src/ospath_linux.c>
    $<$<PLATFORM_ID:Linux>:src/thread_linux.c>
    $<$<PLATFORM_ID:Linux>:src/utf8_linux.c>

    $<$<PLATFORM_ID:Windows>:src/backtrace_win32.c>
    $<$<PLATFORM_ID:Windows>:src/dynlib_win32.c>
    $<$<PLATFORM_ID:Windows>:src/fs_win32.c>
    $<$<PLATFORM_ID:Windows>:src/mfile_win32.c>
    $<$<PLATFORM_ID:Windows>:src/ospath_win32.c>
    $<$<PLATFORM_ID:Windows>:src/thread_win32.c>
    $<$<PLATFORM_ID:Windows>:src/utf8_win32.c>)
target_include_directories (odb-sdk
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:include/odb-sdk>")
target_compile_definitions (odb-sdk
    PRIVATE
        ODBSDK_BUILDING
        _CRT_SECURE_NO_WARNINGS)
target_compile_options (odb-sdk
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter -fpermissive>)
include (ODBTargetProperties)
odb_target_properties (odb-sdk
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
    target_link_libraries (odb-sdk PRIVATE DL::DL)
endif ()

target_link_libraries (odb-sdk
    PRIVATE
        $<$<PLATFORM_ID:Windows>:$<$<BOOL:${ODBSDK_MEM_BACKTRACE}>:Dbghelp>>)

include (FetchContent)

if (ODBSDK_TESTS)
    add_executable (test_odb-sdk
        "tests/env_odbsdk_init.cpp"
        "tests/test_odbsdk_btree.cpp"
        "tests/test_odbsdk_btree_as_set.cpp"
        #"tests/test_odbsdk_fs.cpp"
        "tests/test_odbsdk_log.cpp"
        "tests/test_odbsdk_mem.cpp"
        "tests/test_odbsdk_hm.cpp"
        "tests/test_odbsdk_rb.cpp"
        "tests/test_odbsdk_utf8.cpp"
        "tests/test_odbsdk_utf8_list.cpp"
        "tests/test_odbsdk_vec.cpp")
        #"tests/test_DynamicLibrary.cpp")
    target_link_libraries (test_odb-sdk PUBLIC
        odb-sdk
        gmock
        gmock_main)
    odb_target_properties (test_odb-sdk PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${ODB_BUILD_BINDIR}
        ARCHIVE_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY ${ODB_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${ODB_BUILD_BINDIR})
endif ()

add_subdirectory ("plugins")
#add_subdirectory ("runtime")
add_subdirectory ("runtime-dbp")

###############################################################################
# Installation
###############################################################################

install (
    TARGETS odb-sdk
    EXPORT OpenDarkBASICConfig
    ARCHIVE DESTINATION ${ODB_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${ODB_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${ODB_INSTALL_BINDIR})
install (
    DIRECTORY "include/odb-sdk"
    DESTINATION ${ODB_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/odb-sdk"
    DESTINATION ${ODB_INSTALL_INCLUDEDIR})
install (
    EXPORT OpenDarkBASICConfig
    DESTINATION "share/odb/cmake")
