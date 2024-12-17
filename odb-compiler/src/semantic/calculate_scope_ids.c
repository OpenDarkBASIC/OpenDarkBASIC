#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/semantic/semantic.h"

static void
process_node(
    struct ast* ast, ast_id n, int32_t current_scope, int32_t* scope_counter)
{
    switch (ast_node_type(ast, n))
    {
        /* Functions are comprised of multiple nodes. For scope purposes, we
         * want to handle all nodes as one thing.
         *
         * The funcion declaration (identifier, type) belongs to the current
         * scope, while the parameter list, body and return value belong to the
         * child scope. */
        case AST_FUNC1:
        case AST_FUNC_POLY: {
            ast_id f1 = ast_node_type(ast, n) == AST_FUNC_POLY
                            ? ast->nodes[n].func_poly.func
                            : n;
            ast_id f2 = ast->nodes[f1].func1.func2;
            ast_id f3 = ast->nodes[f2].func2.func3;
            ast_id f4 = ast->nodes[f3].func3.func4;

            ast_id identifier = ast->nodes[f1].func1.identifier;
            ast_id as = ast->nodes[f2].func2.as;
            ast_id paramlist = ast->nodes[f3].func3.paramlist;
            ast_id body = ast->nodes[f4].func4.body;
            ast_id ret = ast->nodes[f4].func4.retval;

            ast->nodes[n].info.scope_id = current_scope;
            ast->nodes[f1].info.scope_id = current_scope;
            ast->nodes[f2].info.scope_id = current_scope;
            ast->nodes[identifier].info.scope_id = current_scope;
            if (as > -1)
                process_node(ast, as, current_scope, scope_counter);

            current_scope = ++(*scope_counter);

            /* f3 and f4 contain the paramlist, body and return value, which
             * belong to the new scope */
            ast->nodes[f3].info.scope_id = current_scope;
            ast->nodes[f4].info.scope_id = current_scope;

            if (paramlist > -1)
                process_node(ast, paramlist, current_scope, scope_counter);
            if (body > -1)
                process_node(ast, body, current_scope, scope_counter);
            if (ret > -1)
                process_node(ast, ret, current_scope, scope_counter);
            break;
        }
        /* Is handled above */
        case AST_FUNC2:
        case AST_FUNC3:
        case AST_FUNC4: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case AST_UDT_DECL: {
            ast_id identifier = ast->nodes[n].udt_decl.type_identifier;
            ast_id members = ast->nodes[n].udt_decl.members;

            ast->nodes[n].info.scope_id = current_scope;

            process_node(ast, identifier, *scope_counter, scope_counter);
            ++(*scope_counter);
            process_node(ast, members, *scope_counter, scope_counter);

            break;
        }

        case AST_BLOCK: {
            /* Help reduce total recursion depth */
            for (; n > -1; n = ast->nodes[n].block.next)
            {
                ast->nodes[n].info.scope_id = current_scope;
                process_node(
                    ast,
                    ast->nodes[n].block.stmt,
                    current_scope,
                    scope_counter);
            }
            break;
        }
        case AST_ARGLIST: {
            /* Help reduce total depth of recursion */
            for (; n > -1; n = ast->nodes[n].arglist.next)
            {
                ast->nodes[n].info.scope_id = current_scope;
                process_node(
                    ast,
                    ast->nodes[n].arglist.expr,
                    current_scope,
                    scope_counter);
            }
            break;
        }
        case AST_PARAMLIST: {
            /* Help reduce total recursion depth */
            for (; n > -1; n = ast->nodes[n].paramlist.next)
            {
                ast->nodes[n].info.scope_id = current_scope;
                process_node(
                    ast,
                    ast->nodes[n].paramlist.param,
                    current_scope,
                    scope_counter);
            }
            break;
        }

        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case AST_END:
        case AST_COMMAND_NAME:
        case AST_COMMAND:
        case AST_TYPELIST:
        case AST_LOAD_PLUGIN:
        case AST_LOAD_COMMAND:
        case AST_ASSIGNMENT:
        case AST_VAR_DECL1:
        case AST_VAR_DECL2:
        case AST_VAR_READ:
        case AST_VAR_WRITE:
        case AST_UDT_INIT:
        case AST_UDT_READ:
        case AST_UDT_WRITE:
        case AST_DIM_DECL1:
        case AST_DIM_DECL2:
        case AST_DIM_READ:
        case AST_DIM_WRITE:
        case AST_PARAM:
        case AST_IDENTIFIER:
        case AST_BINOP:
        case AST_UNOP:
        case AST_COND:
        case AST_SELECT:
        case AST_CASELIST:
        case AST_CASE:
        case AST_COND_BRANCHES:
        case AST_LOOP1:
        case AST_LOOP2:
        case AST_LOOP_FOR1:
        case AST_LOOP_FOR2:
        case AST_LOOP_FOR3:
        case AST_LOOP_CONT:
        case AST_LOOP_EXIT:
        case AST_FUNC_EXIT:
        case AST_FUNC_CALL:
        case AST_CALL_LIKE:
        case AST_CONTAINER_WRITE:
        case AST_BOOLEAN_LITERAL:
        case AST_BYTE_LITERAL:
        case AST_WORD_LITERAL:
        case AST_DWORD_LITERAL:
        case AST_INTEGER_LITERAL:
        case AST_DOUBLE_INTEGER_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_DOUBLE_LITERAL:
        case AST_STRING_LITERAL:
        case AST_CAST:
        case AST_AS_TYPE:
        case AST_AS_EXPR:
        case AST_AS_AUTO:
        case AST_AS_UDT: {
            ast_id left = ast->nodes[n].base.left;
            ast_id right = ast->nodes[n].base.right;

            ast->nodes[n].info.scope_id = current_scope;

            if (left > -1)
                process_node(ast, left, current_scope, scope_counter);
            if (right > -1)
                process_node(ast, right, current_scope, scope_counter);
            break;
        }
    }
}

static int
calculate_scope_ids(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct udt_storage*  udts,
    const struct global_symbols*      globals)
{
    struct ast* ast = tus[tu_id];
    int32_t     scope_counter = 0;

    process_node(ast, ast->root, 0, &scope_counter);

    return 0;
}

static const struct semantic_check* depends[] = {
    /* These are all passes that add new nodes to the AST, and don't want to
       care about calculating scope IDs */
    &semantic_loop_for,
    &semantic_loop_cont,
    &semantic_resolve_command_names,
    NULL};

const struct semantic_check semantic_calculate_scope_ids
    = {calculate_scope_ids, depends, "calculate_scope_ids"};
