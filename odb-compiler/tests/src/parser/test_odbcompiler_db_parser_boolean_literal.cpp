#include "odb-compiler/ast/ast.h"
#include "odb-compiler/tests/DBParserTestHarness.hpp"
#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_literal_bool

using namespace testing;

class NAME : public DBParserTestHarness
{
public:
};

TEST_F(NAME, bool_true)
{
    ASSERT_THAT(parse("#constant a true\n"), Eq(0));

    ASSERT_THAT(ast.node_count, Eq(4));
    ASSERT_THAT(ast.nodes[0].info.type, Eq(AST_BLOCK));
    ASSERT_THAT(ast.nodes[0].block.next, Eq(-1));

    int stmt = ast.nodes[0].block.stmt;
    ASSERT_THAT(stmt, Gt(0));
    ASSERT_THAT(ast.nodes[stmt].info.type, Eq(AST_CONST_DECL));

    int ident = ast.nodes[stmt].const_decl.identifier;
    ASSERT_THAT(ident, Gt(0));
    ASSERT_THAT(ast.nodes[ident].info.type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[ident].identifier.name.off, Eq(10));
    ASSERT_THAT(ast.nodes[ident].identifier.name.len, Eq(1));

    int expr = ast.nodes[stmt].const_decl.expr;
    ASSERT_THAT(expr, Gt(0));
    ASSERT_THAT(ast.nodes[expr].info.type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[expr].boolean_literal.is_true, IsTrue());
}

TEST_F(NAME, bool_false)
{
    ASSERT_THAT(parse("#constant a false\n"), Eq(0));
    
    ASSERT_THAT(ast.node_count, Eq(4));
    ASSERT_THAT(ast.nodes[0].info.type, Eq(AST_BLOCK));
    ASSERT_THAT(ast.nodes[0].block.next, Eq(-1));

    int stmt = ast.nodes[0].block.stmt;
    ASSERT_THAT(stmt, Gt(0));
    ASSERT_THAT(ast.nodes[stmt].info.type, Eq(AST_CONST_DECL));

    int ident = ast.nodes[stmt].const_decl.identifier;
    ASSERT_THAT(ident, Gt(0));
    ASSERT_THAT(ast.nodes[ident].info.type, Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast.nodes[ident].identifier.annotation, Eq(TA_NONE));
    ASSERT_THAT(ast.nodes[ident].identifier.name.off, Eq(10));
    ASSERT_THAT(ast.nodes[ident].identifier.name.len, Eq(1));

    int expr = ast.nodes[stmt].const_decl.expr;
    ASSERT_THAT(expr, Gt(0));
    ASSERT_THAT(ast.nodes[expr].info.type, Eq(AST_BOOLEAN_LITERAL));
    ASSERT_THAT(ast.nodes[expr].boolean_literal.is_true, IsFalse());
}

TEST_F(NAME, two_statements)
{
    ASSERT_THAT(parse("#constant a false\n#constant b true\n"), Eq(0));
}
