* DBPro engine integration:
    * Keywords are stored in a string table. Use [libpe](https://github.com/merces/libpe) to extract and parse these to get type info.
    * Looking at DBPro compiler source, it seems 3 functions (print, input and something else) are special cases (as they're variadic args)
    * Compiler should:
        * Get keywords from DLL string tables
        * generate `dllimport` function declarations in the LLVM IR
        * call those functions when generating IR for commands.
    * KW ini files can probably be ignored in place of string tables, due to lack of type information.
* Clang Format file?