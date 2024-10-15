#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_variable_declaration

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, explicit_integer_declaration)
{
    const char* source = "a AS INTEGER";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(0, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

