#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
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
                ast_id    rvalue = ast->nodes[n].assignment.expr;
                ast_id    lvalue = ast->nodes[n].assignment.lvalue;
                enum type source_type = ast->nodes[rvalue].info.type_info;
                enum type target_type = ast->nodes[lvalue].info.type_info;

                if (source_type != target_type)
                {
                }
            }
            break;

            case AST_COMMAND: {
                int                      i;
                cmd_id                   cmd_id = ast->nodes[n].cmd.id;
                ast_id                   arglist = ast->nodes[n].cmd.arglist;
                struct param_types_list* params
                    = cmds->param_types->data[cmd_id];

                for (i = 0; i != params->count;
                     ++i, arglist = ast->nodes[arglist].arglist.next)
                {
                    ast_id    arg = ast->nodes[arglist].arglist.expr;
                    enum type arg_type = ast->nodes[arg].info.type_info;
                    enum type param_type = params->data[i].type;

                    if (arg_type != param_type)
                    {
                        ast_id cast = ast_cast(
                            ast,
                            arg,
                            param_type,
                            ast->nodes[arg].info.location);
                        if (cast < -1)
                            return -1;
                        ast->nodes[arglist].arglist.expr = cast;
                    }
                }
            }
            break;

            case AST_IDENTIFIER: break;

            case AST_BINOP: break;

            case AST_UNOP: {
                ast_id    expr = ast->nodes[n].unop.expr;
                enum type target_type = ast->nodes[n].info.type_info;

                if (ast->nodes[expr].info.type_info != target_type)
                {
                    ast_id cast = ast_cast(
                        ast, expr, target_type, ast->nodes[expr].info.location);
                    if (cast < -1)
                        return -1;

                    ast->nodes[n].unop.expr = cast;
                }
            }
            break;

            case AST_COND:
            case AST_COND_BRANCH: break;

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
    = {&semantic_type_check_and_cast, &semantic_resolve_cmd_overloads, NULL};
const struct semantic_check semantic_insert_explicit_type_casts
    = {depends, insert_explicit_type_casts};
