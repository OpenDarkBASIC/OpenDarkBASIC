OpenDarkBASIC
=============

This project is a modern re-implementation of the DarkBASIC Professional language and SDK. It consists of a compiler and a runtime. The compiler is built using FLEX and BISON to parse the language into an AST and uses LLVM for code generation. The runtime provides common runtime functionality and a framework for plugins to register commands.

OpenDarkBASIC supports both the original DarkBASIC Pro SDK and the ODB SDK (reimplementation). Of course, the original SDK will only work on Windows.

Building
========

You will need to install following dependencies:
  + CMake 3.13 or later
  + FLEX 2.6 or later
  + BISON 3.7 or later
  + A C++17 compliant compiler
  + LLVM 10.0 or later

For the Windows peeps out there, you can get up to date FLEX and BISON binaries from [here](https://github.com/lexxmark/winflexbison). You can unzip the release anywhere you want (I put it under ```C:\Program Files (x86)```). To get CMake to find them, you have to add the path to the executables to your PATH.

Mac OS users will want to get up to date versions of cmake, llvm, bison and flex using homebrew:
```sh
brew install cmake llvm bison flex
```

And then add the paths to llvm, bison and flex to ```CMAKE_PREFIX_PATH``` when calling cmake:
```sh
cmake -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/llvm/bin:/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin" ../
```

By default the project is built in release mode. If you want to develop on it, you will want to set it to debug mode.

```sh
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../
cmake --build . -- -j $(nproc)
```

On Windows you can specify the arch using the ```-A``` option to CMake if they're using VS2019 or later:
```sh
cmake -A x64 ../
```

If you're using an earlier version of Visual Studio, then the architecture is an argument to the ```-G``` option. You can type ```cmake --help``` to list all available generators.
```sh
cmake -G "Visual Studio 14 2015 Win64" ../
```

Other interesting CMake options:

| Option                               | Default | Description                                                                 |
| ------------------------------------ |:-------:| ----------------------------------------------------------------------------|
| ODBCOMPILER_LIB_TYPE                 | SHARED  | Build odbc as either SHARED or STATIC                                       |
| ODBCOMPILER_BISON_COUNTER_EXAMPLES   | OFF     | Provide counter examples when sr/rr conflicts occur in the grammar          |
| ODBCOMPILER_DOT_EXPORT               | ON      | Enable Graphviz DOT export capability. Unit tests will also export all ASTs |
| ODBCOMPILER_VERBOSE_BISON            | OFF     | Makes the bison very noisy                                                  |
| ODBCOMPILER_VERBOSE_FLEX             | OFF     | Output every token to stderr                                                |
| ODBCOMPILER_TESTS                    | ON      | Build unit tests                                                            |
| ODBCOMPILER_LLVM_ENABLE_SHARED_LIBS  | OFF     | Link with a shared library build of LLVM                                    |
| ODBSDK_LIB_TYPE                      | SHARED  | Build the SDK library either as SHARED or STATIC

#### LLVM

#### Linux

LLVM can usually be installed directly from your distributions repositories. For example, Ubuntu users can simply install `llvm-dev`, and CMake will detect it.
It can also be [build from source](#building-from-source).

#### macOS

Untested. Binaries seem to be available [here](https://github.com/llvm/llvm-project/releases/tag/llvmorg-10.0.0), so that may work. If not, you could [build from source](#building-from-source).

#### Windows

Unfortunately, development binaries don't exist for Windows, so you'll need to [build from source](#building-from-source).

#### Building from source
If you want a quick list of instructions to build LLVM from source with the minimum required components, follow these instructions:

1. `git clone https://github.com/llvm/llvm-project -b llvmorg-11.0.0` (for LLVM 11.0)
1. `cd llvm-project`
1. `mkdir build && cd build`
1. `cmake ../llvm -DCMAKE_INSTALL_PREFIX=./install -DLLVM_INCLUDE_TESTS=0 -DLLVM_INCLUDE_BENCHMARKS=0 -DLLVM_TARGETS_TO_BUILD="ARM;X86;AArch64" -DLLVM_ENABLE_PROJECTS="lld"`
    * If on Windows, add these options: `-DLLVM_COMPILER_JOBS=$(nproc) -Thost=x64`
    * If on Linux, it is recommended to use ninja build by adding this option: `-GNinja`
1. `cmake --build . --target install` (this took about 15 minutes on a Ryzen 2700X)
1. LLVM binaries will be installed to `path/to/llvm-project/build/install`.
    * Pass `-DLLVM_DIR=path/to/llvm-project/build/install/lib/cmake/llvm` to CMake when configuring OpenDarkBASIC to point it to your binaries.

Running
=======

You can run the unit tests by executing:
```sh
cd build/bin
./odbc_tests
```

There is some sample DarkBASIC code in the folder ```dba-sources``` in the root directory which you can try and compile.

In this example we'll parse the file ```iced.dba```, which is an old DarkBASIC Classic sample clocking in at around 1.3k lines of code. Here's the full command required to generate an executable:

```sh
cd build/bin
./odbc \
    --parse-dba ../../dba-sources/iced.dba \
    -o iced.exe
```

Most likely, though, this will fail because the project is still under heavy development. You can, however, try to output one of the intermediate stages. For example, you could generate a graph of the AST using Graphviz:

```sh
cd build/bin
./odbc \
    --parse-dba ../../dba-sources/iced.dba \
    --dump-ast-dot \
    | dot -Tpdf > out.pdf
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

