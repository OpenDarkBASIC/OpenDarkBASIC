#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static void
process_node(
    struct ast* ast, ast_id n, int32_t current_scope, int32_t* scope_counter)
{

    /* The only node that creates new scopes (for now) is AST_FUNC. The func
     * itself belongs to the parent scope, but its parameters, body and return
     * expression belong to a new scope. Unfortunately, the parameter list is
     * contained in an adjacent node, so we also have to include the recurse
     * logic here to get the paramter, body, and return scopes to be identical.
     */
    switch (ast_node_type(ast, n))
    {
        case AST_FUNC:
        case AST_FUNC_TEMPLATE: {
            ast_id decl = ast->nodes[n].func.decl;
            ast_id def = ast->nodes[n].func.def;
            ast_id identifier = ast->nodes[decl].func_decl.identifier;
            ast_id paramlist = ast->nodes[decl].func_decl.paramlist;
            ast_id body = ast->nodes[def].func_def.body;
            ast_id ret = ast->nodes[def].func_def.retval;

            ast->nodes[n].info.scope_id = current_scope;
            ast->nodes[decl].info.scope_id = current_scope;
            ast->nodes[def].info.scope_id = current_scope;
            ast->nodes[identifier].info.scope_id = current_scope;

            current_scope = ++(*scope_counter);

            if (paramlist > -1)
                process_node(ast, paramlist, current_scope, scope_counter);
            if (body > -1)
                process_node(ast, body, current_scope, scope_counter);
            if (ret > -1)
                process_node(ast, ret, current_scope, scope_counter);
            break;
        }

        case AST_BLOCK: {
            /* Help reduce total depth of recursion */
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
            /* Help reduce total depth of recursion */
            for (; n > -1; n = ast->nodes[n].paramlist.next)
            {
                ast->nodes[n].info.scope_id = current_scope;
                process_node(
                    ast,
                    ast->nodes[n].paramlist.identifier,
                    current_scope,
                    scope_counter);
            }
            break;
        }

        case AST_GC:
        case AST_END:
        case AST_COMMAND:
        case AST_ASSIGNMENT:
        case AST_IDENTIFIER:
        case AST_BINOP:
        case AST_UNOP:
        case AST_COND:
        case AST_COND_BRANCHES:
        case AST_LOOP:
        case AST_LOOP_BODY:
        case AST_LOOP_FOR1:
        case AST_LOOP_FOR2:
        case AST_LOOP_FOR3:
        case AST_LOOP_CONT:
        case AST_LOOP_EXIT:
        case AST_FUNC_DECL:
        case AST_FUNC_DEF:
        case AST_FUNC_EXIT:
        case AST_FUNC_OR_CONTAINER_REF:
        case AST_FUNC_CALL:
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
        case AST_SCOPE: {
            ast_id left = ast->nodes[n].base.left;
            ast_id right = ast->nodes[n].base.right;

            ast->nodes[n].info.scope_id = current_scope;

            if (left > -1)
                process_node(ast, left, current_scope, scope_counter);
            if (right > -1)
                process_node(ast, right, current_scope, scope_counter);
        }
        break;
    }
}

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
static void
reset_scope_ids(struct ast* ast)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        ast->nodes[n].info.scope_id = -1;
}

static void
check_scope_ids(struct ast* ast)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
    {
        ODBUTIL_DEBUG_ASSERT(
            ast->nodes[n].info.scope_id > -1,
            log_semantic_err(
                "Node %d of type %d has no scope ID\n",
                n,
                ast_node_type(ast, n)));
    }
}
#endif

static int
calculate_scope_ids(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    struct ast* ast = tus[tu_id];
    int32_t     scope_counter = 0;

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    reset_scope_ids(ast);
#endif

    process_node(ast, ast->root, 0, &scope_counter);

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    check_scope_ids(ast);
#endif

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_calculate_scope_ids
    = {calculate_scope_ids, depends, "calculate_scope_ids"};
