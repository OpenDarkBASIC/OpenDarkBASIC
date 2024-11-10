# OpenDarkBASIC

This  project  is a modern  re-implementation  of  the  DarkBASIC  Professional
language and SDK.  It  consists  of  a  compiler and a runtime. The compiler is
built using FLEX and BISON to parse the language into an AST and  uses LLVM for
code  generation. The runtime  provides  common  runtime  functionality  and  a
framework for plugins to register commands.

OpenDarkBASIC  supports  both  the  original DarkBASIC Pro SDK and the ODB  SDK
(reimplementation). Of course,  the  original  SDK  will  only work on Windows.

## Dependencies

You will need to install following dependencies:
  + CMake 3.28 or later
  + FLEX 2.6 or later
  + BISON 3.8 or later
  + A C compiler
  + LLVM 18.0 or later
  + LLD 18.0 or later

### Linux

LLVM can usually be installed directly from  your  distributions  repositories.
For example, Ubuntu users can simply install  `llvm-dev`, and CMake will detect
it.

It can also be [built from source](#building-llvm).

### Windows

For the Windows peeps out there, you can get up to date FLEX and BISON binaries
from  [here](https://github.com/lexxmark/winflexbison).  You  can   unzip   the
release  anywhere  you want (I put it under ```C:\GnuWin32```). To get CMake to
find  them, you have to  add  the  path  to  both  executables  to  your  PATH.

Install the Microsoft C/C++ (MSVC). You can get it by installing Visual Studio.

Unfortunately,  development  binaries of LLVM/LLD don't exist for  Windows,  so
you'll need to [build from source](#building-llvm).

### Mac

Mac  OS  users will want to get up to date versions of cmake, llvm,  bison  and
flex using homebrew:

```sh
brew install cmake llvm bison flex
```

And then add the paths to llvm, bison and  flex to ```CMAKE_PREFIX_PATH``` when
calling cmake:

```sh
cmake -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin"
```

## Building LLVM

If you  want  a  quick  list of instructions to build LLVM from source with the
minimum required components,  follow  these  instructions. The commands are for
Windows, but are very similar on other platforms.

Debug build:

```sh
git clone https://github.com/llvm/llvm-project -b llvmorg-18.1.4 thirdparty/llvm-project
cmake \
  -S thirdparty/llvm-project/llvm \
  -B build-llvm-debug -A x64 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX:PATH="%CD%/build-llvm-debug/dist" \
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_OPTIMIZED_TABLEGEN=ON \
  -DLLVM_TARGETS_TO_BUILD="ARM;X86;AArch64;WebAssembly" \
  -DLLVM_ENABLE_PROJECTS="clang;lldb;lld"
cmake --build build-llvm-debug --target install
```

Release build:

```sh
git clone https://github.com/llvm/llvm-project -b llvmorg-18.1.4 thirdparty/llvm-project
cmake \
  -S thirdparty/llvm-project/llvm \
  -B build-llvm-release -A x64 \
  -DCMAKE_INSTALL_PREFIX:PATH="%CD%/build-llvm-release/dist" \
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_TARGETS_TO_BUILD="ARM;X86;AArch64;WebAssembly" \
  -DLLVM_ENABLE_PROJECTS="clang;lldb;lld"
cmake --build build-llvm-release --config Release --target install
```

Note: lldb requires clang to be enabled as  well. If not building lldb, you can
remove clang.

LLVM binaries will be installed to ```build-llvm-<config>/dist```.

## Building

By default the project is built in release mode. If you want to develop on  it,
you will want  to  set  it to debug mode. Here is the recommended CMake command
for a debug build:

```sh
cmake \
  -A x64 \
  -G Ninja \
  -S . \
  -B build-Debug \
  -DCMAKE_BUILD_TYPE=Debug \
  # If you didn't add FLEX/BISON to your PATH, you can add it here \
  -DCMAKE_PREFIX_PATH:PATH="/path/to/flex:/path/to/bison" \
  -DLLVM_DIR=/usr/lib/llvm/18/lib64/cmake/llvm \
  -DLLD_DIR=/usr/lib/llvm/18/lib64/cmake/lld
cmake --build build-Debug --parallel $(nproc)
```

### Cross compilation and the DarkBASIC SDK

On   Windows,  a  32-bit  DarkBASIC   runtime   is   built   automatically   if
```-DDBP_SDK=ON``` (default). The compiler itself will be a 64-bit  executable.
This allows you to use the original DarkBASIC SDK.

On  Mac and Linux, only 64-bit executables are built, and the DarkBASIC SDK  is
not  supported.  If  you wish to cross-compile DarkBASIC executables, you  will
have to build the 32-bit  runtime  manually  on  Windows and copy the resulting
files over to your Mac/Linux machine.

If you want to build Linux/Mac executables from  Windows, you will also have to
manually compile the linux runtimes and copy them over.

### Build options

A non-exhaustive list of build options:

| Option                               | Default | Description                                                                 |
| ------------------------------------ |:-------:| ----------------------------------------------------------------------------|
| ODB_UTIL                             | ON      | The utility library is used by all subprojects.                             |
| ODB_UTIL_i386                        | ON      | Also build a 32-bit version of the util library (required for DarkBASIC)    |
| ODB_COMPILER                         | ON      | Build the compiler library. Required by the CLI, editor and LSP.            |
| ODB_CLI                              | ON      | Build the command line interface to the compiler.                           |
| ODB_SDK                              | ON      | Build the OpenDarkBASIC reimplementation of the DarkBASIC SDK.              |
| DBP_SDK                              | ON      | Build the runtime for interfacing with the original DarkBASIC SDK.          |
| ODB_EDITOR                           | OFF     | Build the editor (requires GTK4)                                            |
| ODB_PROFILING                        | OFF     | Enable profiling support (-pg)                                              |
| ODB_TESTS                            | ON      | Build the unit tests.                                                       |
| ODB_WERROR                           | Debug   | Treat warnings as errors. Enabled in Debug mode.                            |
| ODBCOMPILER_LIB_TYPE                 | SHARED  | Build the compiler library either as SHARED or STATIC                       |
| ODBCOMPILER_BISON_COUNTER_EXAMPLES   | OFF     | Provide counter examples when sr/rr conflicts occur in the grammar          |
| ODBCOMPILER_UPDATE_BUILDINFO         | Release | Re-generate the buildinfo.h header. Can get annoying when developing.       |
| ODBCOMPILER_DOT_EXPORT               | ON      | Enable Graphviz DOT export capability. Unit tests will also export all ASTs |
| ODBCOMPILER_VERBOSE_BISON            | OFF     | Makes the bison very noisy                                                  |
| ODBCOMPILER_VERBOSE_FLEX             | OFF     | Output every token to stderr                                                |
| ODBCOMPILER_TESTS                    | ON      | Build unit tests                                                            |
| ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS  | OFF     | Link with a shared library build of LLVM                                    |
| ODBUTIL_LIB_TYPE                     | SHARED  | Build the SDK library either as SHARED or STATIC

Running
=======

You can run the unit tests by executing:
```sh
(cd build/bin && ./odb-tests)
```

There is some sample DarkBASIC code in the folder ```dba-sources``` in the root directory which you can try and compile.

In this example we'll parse the file ```iced.dba```, which is an old DarkBASIC Classic sample clocking in at around 1.3k lines of code. Here's the full command required to generate an executable:

```sh
(cd build/bin &&
./odbc \
    --parse-dba ../../dba-sources/iced.dba \
    -o iced.exe)
```

Most likely, though, this will fail because the project is still under heavy development. You can, however, try to output one of the intermediate stages. For example, you could generate a graph of the AST using Graphviz:

```sh
(cd build/bin &&
./odbc \
    --parse-dba ../../dba-sources/iced.dba \
    --dump-ast-dot \
    | dot -Tpdf > out.pdf)
```

Now you can open ```out.pdf``` with your favorite PDF viewer and see a visual representation of the program's structure.

Fuzzing
=======

```sh
mkdir build-afl
sudo mount -t tmpfs -o size=16384m afl-ramdisk build-afl
cd build-afl
CC=afl-gcc CXX=afl-g++ AS=afl-as cmake -DCMAKE_BUILD_TYPE=Release -DODBCOMPILER_TESTS=OFF -DODBCOMPILER_LLVM_ENABLE_SHARED_LIBS=ON ../
make -j$(nproc)
cd bin
cp ../../scripts/fuzz_odbc.sh .
./fuzz_odbc.sh
```

