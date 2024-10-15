#pragma once

#include <initializer_list>

extern "C" {
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/semantic/type.h"
#include "odb-compiler/sdk/cmd_list.h"

struct ast;
struct mutex;
struct symbol_table;
struct plugin_list;
}

struct DBParserHelper
{
    DBParserHelper();
    ~DBParserHelper();

    int
    parse(const char* code);
    int
    semantic(const struct semantic_check* check);
    int
    addCommand(const char* name);
    int
    addCommand(type return_type, const char* name);
    int
    addCommand(
        type                        return_type,
        const char*                 name,
        std::initializer_list<type> param_types);

    struct plugin_list*  plugins;
    struct cmd_list      cmds;
    struct symbol_table* symbols;
    struct db_parser     p;
    struct db_source     src;
    struct ast*          ast;
    struct mutex*        ast_mutex;
};
