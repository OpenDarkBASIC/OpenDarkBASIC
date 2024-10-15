#pragma once

#include <initializer_list>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_parser.h"

struct mutex;
struct symbol_table;
struct plugin_list;
}

struct DBParserHelper
{
    DBParserHelper();
    ~DBParserHelper();

    int parse(const char* code);
    int semantic(const struct semantic_check* check);
    int addCommand(const char* name);
    int addCommand(type return_type, const char* name);
    int addCommand(type return_type, const char* name, std::initializer_list<type> param_types);

    struct plugin_list*  plugins;
    struct cmd_list      cmds;
    struct symbol_table* symbols;
    struct db_parser     p;
    struct db_source     src;
    struct ast           ast;
    struct mutex*        ast_mutex;
};
