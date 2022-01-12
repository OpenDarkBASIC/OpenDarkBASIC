set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LLD_BUILT_STANDALONE TRUE)

find_program(LLVM_CONFIG_PATH NAMES llvm-config llvm-config-${LLVM_VERSION_MAJOR})
if(NOT LLVM_CONFIG_PATH)
    message(FATAL_ERROR "llvm-config not found: specify LLVM_CONFIG_PATH")
endif()

execute_process(COMMAND "${LLVM_CONFIG_PATH}"
    "--obj-root"
    "--includedir"
    "--cmakedir"
    "--src-root"
    RESULT_VARIABLE HAD_ERROR
    OUTPUT_VARIABLE LLVM_CONFIG_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(HAD_ERROR)
    message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
endif()

string(REGEX REPLACE "[ \t]*[\r\n]+[ \t]*" ";" LLVM_CONFIG_OUTPUT "${LLVM_CONFIG_OUTPUT}")

list(GET LLVM_CONFIG_OUTPUT 0 OBJ_ROOT)
list(GET LLVM_CONFIG_OUTPUT 1 MAIN_INCLUDE_DIR)
list(GET LLVM_CONFIG_OUTPUT 2 LLVM_CMAKE_PATH)
list(GET LLVM_CONFIG_OUTPUT 3 MAIN_SRC_DIR)

set(LLVM_OBJ_ROOT ${OBJ_ROOT} CACHE PATH "path to LLVM build tree")
set(LLVM_MAIN_INCLUDE_DIR ${MAIN_INCLUDE_DIR} CACHE PATH "path to llvm/include")
set(LLVM_MAIN_SRC_DIR ${MAIN_SRC_DIR} CACHE PATH "Path to LLVM source tree")

file(TO_CMAKE_PATH ${LLVM_OBJ_ROOT} LLVM_BINARY_DIR)
file(TO_CMAKE_PATH ${LLVM_CMAKE_PATH} LLVM_CMAKE_PATH)

if(NOT EXISTS "${LLVM_CMAKE_PATH}/LLVMConfig.cmake")
    message(FATAL_ERROR "LLVMConfig.cmake not found")
endif()
include("${LLVM_CMAKE_PATH}/LLVMConfig.cmake")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_PATH}")

set(PACKAGE_VERSION "${LLVM_PACKAGE_VERSION}")
include_directories("${LLVM_BINARY_DIR}/include" ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${LLVM_LIBDIR_SUFFIX})
set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
find_program(LLVM_TABLEGEN_EXE "llvm-tblgen" ${LLVM_TOOLS_BINARY_DIR} NO_DEFAULT_PATH)

include(AddLLVM)
include(TableGen)
include(HandleLLVMOptions)
if(${LLVM_VERSION} VERSION_GREATER_EQUAL 13.0.0)
    include(GetErrcMessages)
endif ()
if(${LLVM_VERSION} VERSION_GREATER_EQUAL 11.0.0)
    include(CheckAtomic)
endif()

if(LLVM_HAVE_LIBXAR)
    set(XAR_LIB xar)
endif()

if(NOT WIN32)
    set(LLVM_LINK_LLVM_DYLIB ${ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS})
endif()

function(export_executable_symbols_for_plugins target)
endfunction()

FetchContent_Declare(
    lld
    URL https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/lld-${LLVM_VERSION}.src.tar.xz
)
FetchContent_MakeAvailable(lld)

target_compile_features(lldCommon PUBLIC cxx_std_17)
target_compile_features(lldCore PUBLIC cxx_std_17)
target_compile_features(lldCOFF PUBLIC cxx_std_17)
target_compile_features(lldELF PUBLIC cxx_std_17)
target_compile_features(lldMachO PUBLIC cxx_std_17)
