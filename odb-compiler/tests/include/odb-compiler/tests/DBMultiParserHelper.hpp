#pragma once

#include <vector>

extern "C" {
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/type.h"

struct ast;
struct mutex;
struct globals;
struct plugin_list;
}

struct DBMultiParserHelper
{
    DBMultiParserHelper();
    ~DBMultiParserHelper();

    virtual int
    parse(const char* code);

    struct plugin_list* plugins;
    struct ospath_list* filenames;
    std::vector<utf8>   sources;
    std::vector<ast*>   asts;
    struct db_parser    p;
    struct cmd_list     cmds;
    struct globals*     globals;
};
