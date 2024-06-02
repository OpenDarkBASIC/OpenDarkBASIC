#pragma once

#include <initializer_list>
#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/cmd_list.h"
}

class DBParserHelper : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    int parse(const char* code);
    int addCommand(const char* name);
    int addCommand(type return_type, const char* name);
    int addCommand(type return_type, const char* name, std::initializer_list<type> param_types);

    struct plugin_list plugins;
    struct cmd_list    cmds;
    struct db_parser   p;
    struct db_source   src;
    struct ast         ast;
};
