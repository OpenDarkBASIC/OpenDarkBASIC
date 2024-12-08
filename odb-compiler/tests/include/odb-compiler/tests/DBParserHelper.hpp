#pragma once

#include <initializer_list>

extern "C" {
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/type.h"
#include "odb-compiler/semantic/udt.h"

struct ast;
struct mutex;
struct globals;
struct plugin_list;
}

struct DBParserHelper
{
    DBParserHelper();
    ~DBParserHelper();

    virtual int
    parse(const char* code);
    int semantic(const struct semantic_check* check);
    int addCommand(const char* name);
    int addCommand(enum primitive_type return_type, const char* name);
    int addCommand(union type, const char* name);
    int addCommand(
        union type                        return_type,
        const char*                       name,
        std::initializer_list<union type> param_types);
    int addCommand(
        enum primitive_type                        return_type,
        const char*                                name,
        std::initializer_list<enum primitive_type> param_types);

    struct plugin_list* plugins;
    struct cmd_list     cmds;
    struct udt_storage  udts;
    struct globals*     globals;
    struct db_parser    p;
    struct ospath_list* filenames;
    struct utf8         src;
    struct ast*         ast;
    struct mutex*       ast_mutex;
    struct mutex*       cmd_list_mutex;

private:
    void
    writeAST();
};
