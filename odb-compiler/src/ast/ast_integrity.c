#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-util/log.h"

static int
count_nodes_recurse(const struct ast* ast, ast_id n)
{
    ast_id count = 1;
    if (ast->nodes[n].base.left > -1)
        count += count_nodes_recurse(ast, ast->nodes[n].base.left);
    if (ast->nodes[n].base.right > -1)
        count += count_nodes_recurse(ast, ast->nodes[n].base.right);

    /* Special case with for-loops, they have more than 2 children */
    if (ast->nodes[n].info.node_type == AST_LOOP
        && ast->nodes[n].loop.loop_for > -1)
    {
        count += count_nodes_recurse(ast, ast->nodes[n].loop.loop_for);
    }
    if (ast->nodes[n].info.node_type == AST_LOOP_FOR)
    {
        if (ast->nodes[n].loop_for.step > -1)
            count += count_nodes_recurse(ast, ast->nodes[n].loop_for.step);
        if (ast->nodes[n].loop_for.next > -1)
            count += count_nodes_recurse(ast, ast->nodes[n].loop_for.next);
    }

    return count;
}

static void
print_node(const char* name, int depth)
{
    int i;
    log_note("[ast] ", "%s", "");
    for (i = 0; i != depth; ++i)
        log_raw("  ");
    log_raw("%s\n", name);
}

static void
print_subtree(const struct ast* ast, ast_id n, int depth)
{
    switch (ast->nodes[n].info.node_type)
    {
            /* clang-format off */
        case AST_GC: print_node("GC", depth); break;
        case AST_BLOCK: print_node("BLOCK", depth); break;
        case AST_END: print_node("END", depth); break;
        case AST_ARGLIST: print_node("ARGLIST", depth); break;
        case AST_PARAMLIST: print_node("PARAMLIST", depth); break;
        case AST_COMMAND: print_node("COMMAND", depth); break;
        case AST_ASSIGNMENT: print_node("ASSIGNMENT", depth); break;
        case AST_IDENTIFIER: print_node("IDENTIFIER", depth); break;
        case AST_BINOP: print_node("BINOP", depth); break;
        case AST_UNOP: print_node("UNOP", depth); break;
        case AST_COND: print_node("COND", depth); break;
        case AST_COND_BRANCH: print_node("COND_BRANCH", depth); break;
        case AST_LOOP: print_node("LOOP", depth); break;
        case AST_LOOP_FOR: print_node("LOOP_FOR", depth); break;
        case AST_LOOP_CONT: print_node("LOOP_CONT", depth); break;
        case AST_LOOP_EXIT: print_node("LOOP_EXIT", depth); break;
        case AST_FUNC: print_node("FUNC", depth); break;
        case AST_FUNC_DECL: print_node("FUNC_DECL", depth); break;
        case AST_FUNC_DEF: print_node("FUNC_DEF", depth); break;
        case AST_FUNC_OR_CONTAINER_REF: print_node("FUNC_OR_CONTAINER_REF", depth); break;
        case AST_FUNC_CALL: print_node("FUNC_CALL", depth); break;
        case AST_BOOLEAN_LITERAL: print_node("BOOLEAN_LITERAL", depth); break;
        case AST_BYTE_LITERAL: print_node("BYTE_LITERAL", depth); break;
        case AST_WORD_LITERAL: print_node("WORD_LITERAL", depth); break;
        case AST_DWORD_LITERAL: print_node("DWORD_LITERAL", depth); break;
        case AST_INTEGER_LITERAL: print_node("INTEGER_LITERAL", depth); break;
        case AST_DOUBLE_INTEGER_LITERAL: print_node("DOUBLE_INTEGER_LITERAL", depth); break;
        case AST_FLOAT_LITERAL: print_node("FLOAT_LITERAL", depth); break;
        case AST_DOUBLE_LITERAL: print_node("DOUBLE_LITERAL", depth); break;
        case AST_STRING_LITERAL: print_node("STRING_LITERAL", depth); break;
        case AST_CAST: break; print_node("CAST", depth); break;
            /* clang-format on */
    }

    if (ast->nodes[n].base.left > -1)
        print_subtree(ast, ast->nodes[n].base.left, depth + 1);
    if (ast->nodes[n].base.right > -1)
        print_subtree(ast, ast->nodes[n].base.right, depth + 1);
}

static void
report_unconnected_nodes(const struct ast* ast)
{
    ast_id n;
    for (n = 0; n != ast->count; ++n)
    {
        ast_id parent = ast_find_parent(ast, n);
        if (parent == -1 && n != ast->root)
            print_subtree(ast, n, 0);
    }
}

int
ast_verify_connectivity(const struct ast* ast)
{
    ast_id count = count_nodes_recurse(ast, ast->root);
    if (count != ast->count)
    {
        log_err(
            "[ast] ",
            "%d out of %d nodes reachable from root. Did you forget to call "
            "ast_gc()?\n",
            count,
            ast->count);
        report_unconnected_nodes(ast);
        return -1;
    }

    return 0;
}
