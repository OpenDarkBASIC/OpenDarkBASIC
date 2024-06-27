#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_initialize_undeclared_variables

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, boolean_initialized_to_false)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOLEAN});
    const char* source = "print a";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(-1)) << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var1 = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    ast_id arg1 = ast.nodes[ast.nodes[0].block.next].cmd.arglist;
    ast_id var2 = ast.nodes[arg1].arglist.expr;
    struct utf8_span name1 = ast.nodes[var1].identifier.name;
    struct utf8_span name2 = ast.nodes[var2].identifier.name;
}
