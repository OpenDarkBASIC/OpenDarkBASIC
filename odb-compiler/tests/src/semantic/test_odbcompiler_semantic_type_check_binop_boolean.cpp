#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_boolean

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, and_boolean_doesnt_insert_casts)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_BOOL});
    const char* source
        = "a? = true\n"
          "b? = false\n"
          "print a? and b?\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
    ASSERT_THAT(log(), LogEq(""));

    ast_id block1 = ast->root;
    ast_id block2 = ast->nodes[block1].block.next;
    ast_id block3 = ast->nodes[block2].block.next;
    ast_id block4 = ast->nodes[block3].block.next;
    ASSERT_THAT(block4, Eq(-1));

    ast_id cmd = ast->nodes[block3].block.stmt;
    ast_id args = ast->nodes[cmd].command.arglist;
    ast_id op = ast->nodes[args].arglist.expr;
    ast_id a_var_read = ast->nodes[op].binop.left;
    ast_id b_var_read = ast->nodes[op].binop.right;
    ast_id a_ident = ast->nodes[a_var_read].var_read.identifier;
    ast_id b_ident = ast->nodes[b_var_read].var_read.identifier;
    ASSERT_THAT(ast->nodes[op].binop.op, Eq(BINOP_LOGICAL_AND));
    ASSERT_THAT(ast_node_type(ast, a_var_read), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, b_var_read), Eq(AST_VAR_READ));
    ASSERT_THAT(ast_node_type(ast, a_ident), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_node_type(ast, b_ident), Eq(AST_IDENTIFIER));
    ASSERT_THAT(ast_type_info(ast, a_var_read).primitive, Eq(TYPE_BOOL));
    ASSERT_THAT(ast_type_info(ast, b_var_read).primitive, Eq(TYPE_BOOL));
}
