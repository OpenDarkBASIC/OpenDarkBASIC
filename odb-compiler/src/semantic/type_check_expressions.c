#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
type_check_expressions(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        switch (ast->nodes[n].info.type)
        {
            case AST_BLOCK: break;

            case AST_ARGLIST: break;

            case AST_CONST_DECL: break;

            case AST_COMMAND:
            case AST_ASSIGN: break;

            case AST_IDENTIFIER: break;

            case AST_BOOLEAN_LITERAL: break;

            case AST_BYTE_LITERAL:
            case AST_WORD_LITERAL:
            case AST_INTEGER_LITERAL:
            case AST_DWORD_LITERAL:
            case AST_DOUBLE_INTEGER_LITERAL: break;

            case AST_FLOAT_LITERAL:
            case AST_DOUBLE_LITERAL: break;

            case AST_STRING_LITERAL: break;
        }
    }

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};
const struct semantic_check semantic_type_check_expressions
    = {depends, type_check_expressions};
