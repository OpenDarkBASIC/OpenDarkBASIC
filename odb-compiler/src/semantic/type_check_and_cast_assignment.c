#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
resolve_assignments(
    struct ast*            ast,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
        switch (ast->nodes[n].info.node_type)
        {
            case AST_BLOCK:
            case AST_ARGLIST:
            case AST_CONST_DECL: break;

            /* Assignments themselves are not expressions and thus don't
             * return a value, but the rvalue is often used to determine the
             * type of the lvalue. */
            case AST_ASSIGNMENT: {
            }
            break;

            case AST_COND:
            case AST_COND_BRANCH:
            case AST_LOOP:
            case AST_LOOP_EXIT:
            case AST_COMMAND:
            case AST_IDENTIFIER:
            case AST_BINOP:
            case AST_UNOP:
            case AST_BOOLEAN_LITERAL:
            case AST_BYTE_LITERAL:
            case AST_WORD_LITERAL:
            case AST_DWORD_LITERAL:
            case AST_INTEGER_LITERAL:
            case AST_DOUBLE_INTEGER_LITERAL:
            case AST_FLOAT_LITERAL:
            case AST_DOUBLE_LITERAL:
            case AST_STRING_LITERAL:
            case AST_CAST: break;
        }

    return 0;
}
