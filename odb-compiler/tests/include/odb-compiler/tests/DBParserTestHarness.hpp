#pragma once

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/cmd_list.h"
}

class DBParserTestHarness : public testing::Test
{
public:
    void
    SetUp() override;
    void
    TearDown() override;
    int
    addCommand(const char* name);
    int
    addCommand(enum cmd_param_type return_type, const char* name);
    int
    parse(const char* code);

    struct cmd_list  cmds;
    struct db_parser p;
    struct db_source src;
    struct ast       ast;
};
