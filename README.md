Open DarkBASIC
==============

This project is currently just an excuse for me to learn more about parsing with Flex/Bison. My goal is to successfully parse my old DarkBASIC projects into an AST and interpret it.

It *may* evolve into actually reimplementing the DarkBASIC SDK on top of OpenGL and/or Vulkan, and it *may* evolve into writing a compiler/assembler. Who knows.

Building
========

You will need to install following dependencies:
  + CMake 3.13 or later
  + FLEX
  + BISON 3.2 or later
  + A C++17 compliant compiler
  + LLVM 8.0 or later (optional)

For the Windows peeps out there, you can get up to date FLEX and BISON binaries from [here](https://github.com/lexxmark/winflexbison). You can unzip the release anywhere you want (I put it under ```C:\Program Files (x86)```). To get CMake to find them, you have to add the path to the executables to your PATH.

By default the project is built in release mode. If you want to develop on it, you will want to set it to debug mode.

```sh
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../
cmake --build .
```

The Windows peeps can specify the arch using the ```-A``` option to CMake if they're using VS2019 or later:
```sh
cmake -A x64 ../
```

If you're using an earlier version of Visual Studio, then the architecture is an argument to the ```-G``` option. You can type ```cmake --help``` to list all available generators.
```sh
cmake -G "Visual Studio 14 2015 Win64" ../
```

Other interesting CMake options:

| Option             | Default | Description                                    |
| ------------------ |:-------:| ---------------------------------------------- |
| ODBC_VERBOSE_FLEX  | OFF     | Outputs every token that was matched to stderr |
| ODBC_VERBOSE_BISON | OFF     | Makes the bison very noisy                     |
| ODBC_TESTS         | ON      | Build unit tests                               |
| ODBC_LIB_TYPE      | SHARED  | Build odbc as either SHARED or STATIC          |
| ODBC_DOT_EXPORT    | ON      | Enable Graphviz DOT export capability          |

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

1. `git clone https://github.com/llvm/llvm-project -b llvmorg-10.0.0` (for LLVM 10.0)
1. `cd llvm-project`
1. `mkdir build && cd build`
1. `cmake ../llvm -DCMAKE_INSTALL_PREFIX=$(pwd)/install -DLLVM_INCLUDE_TESTS=0 -DLLVM_INCLUDE_BENCHMARKS=0 -DLLVM_TARGETS_TO_BUILD="X86"`
    * If on Windows, add these options: `-DLLVM_COMPILER_JOBS=$(nproc) -Thost=x64`
    * If on Linux, it is recommended to use ninja build by adding this option: `-GNinja`
1. `cmake --build . --target install` (this took about 15 minutes on a Ryzen 2700X)
1. LLVM binaries will be installed to `path/to/llvm-project/build/install`.
    * Pass `-DLLVM_DIR=path/to/llvm-project/build/install/lib/cmake/llvm` to CMake when configuring OpenDarkBASIC to point it to your binaries.

Running
=======

You can run the unit tests by executing:
```sh
./odb-compiler/odbc_tests
```

There is some sample DarkBASIC code in the folder ```dba-source``` in the root directory which you can try and compile. The CLI interprets the commands you give it immediately (exactly how blender CLI works), so you have to supply the arguments in a particular order.

In this example we'll parse the file ```iced.dba```, which is an old DarkBASIC Classic sample clocking in at around 1.3k lines of code. Here's the full command:

```sh
./odb-compiler/odbc \
    --parse-kw-ini ../odb-sdk/keywords/keywords107.ini ../odb-sdk/keywords/odb-keywords.ini \
    --parse-dba ../dba-sources/iced.dba \
    --dump-ast-json iced.json
```

```--parse-kw-ini```: The compiler needs to know about all of the available keywords in order to parse the source code directly. For now, these are loaded from INI files. ```keywords107.ini``` is the original keyword file for DarkBASIC Professional, and ```odb-keywords.ini``` lists some missing keywords that no longer exist in DBP.

```--parse-dba```: This takes a list of DarkBASIC source files.

```--dump-ast-json```: This will output the AST of the parser to JSON format.

```--emit-llvm```: This flag will emit LLVM IR to stdout.

