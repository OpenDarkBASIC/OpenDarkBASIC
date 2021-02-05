OpenDarkBASIC
=============

This project is a modern re-implementation of the DarkBASIC Professional language and SDK. It consists of a compiler and a runtime. The compiler is built using FLEX and BISON to parse the language into an AST and uses LLVM for code generation. The runtime provides common runtime functionality and a framework for plugins to register commands.

OpenDarkBASIC supports both the original DarkBASIC Pro SDK and the ODB SDK (reimplementation). Of course, the original SDK will only work on Windows.

Building
========

You will need to install following dependencies:
  + CMake 3.13 or later
  + FLEX
  + BISON 3.7 or later
  + A C++17 compliant compiler

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

| Option                             | Default | Description                                                                 |
| ---------------------------------- |:-------:| ----------------------------------------------------------------------------|
| ODBCOMPILER_LIB_TYPE               | SHARED  | Build odbc as either SHARED or STATIC                                       |
| ODBCOMPILER_BISON_COUNTER_EXAMPLES | OFF     | Provide counter examples when sr/rr conflicts occur in the grammar          |
| ODBCOMPILER_DOT_EXPORT             | ON      | Enable Graphviz DOT export capability. Unit tests will also export all ASTs |
| ODBCOMPILER_VERBOSE_BISON          | OFF     | Makes the bison very noisy                                                  |
| ODBCOMPILER_VERBOSE_FLEX           | OFF     | Output every token to stderr                                                |
| ODBCOMPILER_TESTS                  | ON      | Build unit tests                                                            |
| ODBSDK_LIB_TYPE                    | SHARED  | Build the SDK library either as SHARED or STATIC

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

