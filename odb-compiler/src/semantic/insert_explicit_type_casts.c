#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <assert.h>

static int
insert_explicit_type_casts(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
        switch (ast->nodes[n].info.node_type)
        {
            case AST_BLOCK:
            case AST_ARGLIST:
            case AST_CONST_DECL: break;

            case AST_ASSIGNMENT: {
                ast_id    expr = ast->nodes[n].assignment.expr;
                enum type from = ast->nodes[expr].info.type_info;
                enum type to = ast->nodes[n].info.type_info;

                if (from != to)
                {
                    ast_id cast = ast_cast(
                        ast, expr, to, ast->nodes[expr].info.location);
                    if (cast < -1)
                        return -1;

                    ast->nodes[n].assignment.expr = cast;
                }
            }
            break;

            case AST_COMMAND: {
                int                      i;
                cmd_id                   cmd_id = ast->nodes[n].cmd.id;
                ast_id                   arglist = ast->nodes[n].cmd.arglist;
                struct param_types_list* params
                    = vec_get(cmds->param_types, cmd_id);

                for (i = 0; i != vec_count(*params);
                     ++i, arglist = ast->nodes[arglist].arglist.next)
                {
                    ast_id    expr = ast->nodes[arglist].arglist.expr;
                    enum type from = ast->nodes[expr].info.type_info;
                    enum type to = vec_get(*params, i)->type;

                    if (from != to)
                    {
                        ast_id cast = ast_cast(
                            ast, expr, to, ast->nodes[expr].info.location);
                        if (cast < -1)
                            return -1;

                        ast->nodes[arglist].arglist.expr = cast;
                    }
                }
            }
            break;

            case AST_IDENTIFIER: break;

            case AST_BINOP: {
                ast_id    lhs = ast->nodes[n].binop.left;
                ast_id    rhs = ast->nodes[n].binop.right;
                enum type to = ast->nodes[n].info.type_info;

                if (ast->nodes[lhs].info.type_info != to)
                {
                    ast_id cast
                        = ast_cast(ast, lhs, to, ast->nodes[lhs].info.location);
                    if (cast < -1)
                        return -1;

                    ast->nodes[n].binop.left = cast;
                }
                if (ast->nodes[rhs].info.type_info != to)
                {
                    ast_id cast
                        = ast_cast(ast, rhs, to, ast->nodes[rhs].info.location);
                    if (cast < -1)
                        return -1;

                    ast->nodes[n].binop.right = cast;
                }
            }
            break;

            case AST_UNOP: {
                ast_id    expr = ast->nodes[n].unop.expr;
                enum type to = ast->nodes[n].info.type_info;

                if (ast->nodes[expr].info.type_info != to)
                {
                    ast_id cast = ast_cast(
                        ast, expr, to, ast->nodes[expr].info.location);
                    if (cast < -1)
                        return -1;

                    ast->nodes[n].unop.expr = cast;
                }
            }
            break;

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

static const struct semantic_check* depends[]
    = {&semantic_type_check_expressions, &semantic_resolve_cmd_overloads, NULL};
const struct semantic_check semantic_insert_explicit_type_casts
    = {depends, insert_explicit_type_casts};
