#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include <errno.h>
#include <stdio.h>

static void
print_node(const struct ast* ast, FILE* fp, ast_id n, int depth)
{
    int i;
    fprintf(fp, "%d ", n);
    for (i = 0; i != depth; ++i)
        fprintf(fp, " ");

    switch (ast_node_type(ast, n))
    {
            /* clang-format off */
        case AST_GC: fprintf(fp, "GC\n"); break;
        case AST_BLOCK: fprintf(fp, "BLOCK, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_END: fprintf(fp, "END, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_ARGLIST: fprintf(fp, "ARGLIST, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_PARAMLIST: fprintf(fp, "PARAMLIST, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_COMMAND: fprintf(fp, "COMMAND, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_ASSIGNMENT: fprintf(fp, "ASSIGNMENT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_DECL1: fprintf(fp, "VAR_DECL1, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_DECL2: fprintf(fp, "VAR_DECL2, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_READ: fprintf(fp, "VAR_READ, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_VAR_WRITE: fprintf(fp, "VAR_WRITE, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_PARAM: fprintf(fp, "PARAM, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_IDENTIFIER: fprintf(fp, "IDENTIFIER, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_BINOP: fprintf(fp, "BINOP, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_UNOP: fprintf(fp, "UNOP, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_COND: fprintf(fp, "COND, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_COND_BRANCHES: fprintf(fp, "COND_BRANCHES, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP1: fprintf(fp, "LOOP, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP2: fprintf(fp, "LOOP_BODY, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_FOR1: fprintf(fp, "LOOP_FOR1, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_FOR2: fprintf(fp, "LOOP_FOR2, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_FOR3: fprintf(fp, "LOOP_FOR3, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_CONT: fprintf(fp, "LOOP_CONT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_LOOP_EXIT: fprintf(fp, "LOOP_EXIT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_POLY: fprintf(fp, "FUNC POLY, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC1: fprintf(fp, "FUNC1, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC2: fprintf(fp, "FUNC2, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC3: fprintf(fp, "FUNC3, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC4: fprintf(fp, "FUNC4, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_EXIT: fprintf(fp, "FUNC_EXIT, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_OR_CONTAINER_REF: fprintf(fp, "FUNC_OR_CONTAINER_REF, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FUNC_CALL: fprintf(fp, "FUNC_CALL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_BOOLEAN_LITERAL: fprintf(fp, "BOOLEAN_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_BYTE_LITERAL: fprintf(fp, "BYTE_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_WORD_LITERAL: fprintf(fp, "WORD_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_DWORD_LITERAL: fprintf(fp, "DWORD_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_INTEGER_LITERAL: fprintf(fp, "INTEGER_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_DOUBLE_INTEGER_LITERAL: fprintf(fp, "DOUBLE_INTEGER_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_FLOAT_LITERAL: fprintf(fp, "FLOAT_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_DOUBLE_LITERAL: fprintf(fp, "DOUBLE_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_STRING_LITERAL: fprintf(fp, "STRING_LITERAL, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_CAST: fprintf(fp, "CAST, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_AS: fprintf(fp, "AS, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_AS_AUTO: fprintf(fp, "AS, scope=%d\n", ast->nodes[n].info.scope_id); break;
        case AST_TYPE: fprintf(fp, "TYPE, scope=%d\n", ast->nodes[n].info.scope_id); break;
            /* clang-format on */
    }
}

static void
print_subtree(const struct ast* ast, ast_id n, FILE* fp, int depth)
{
    print_node(ast, fp, n, depth);

    /* Print block lists on same depth */
    if (ast_node_type(ast, n) == AST_BLOCK)
    {
        print_subtree(ast, ast->nodes[n].block.stmt, fp, depth + 1);
        if (ast->nodes[n].block.next > -1)
            print_subtree(ast, ast->nodes[n].block.next, fp, depth);
    }
    else
    {
        if (ast->nodes[n].base.left > -1)
            print_subtree(ast, ast->nodes[n].base.left, fp, depth + 1);
        if (ast->nodes[n].base.right > -1)
            print_subtree(ast, ast->nodes[n].base.right, fp, depth + 1);
    }
}
static void
print_ast_nonrecursive(const struct ast* ast, FILE* fp, int depth)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
        print_node(ast, fp, n, depth);
}

static int
max_depth_is_unreasonable(const struct ast* ast, ast_id n, ast_id depth)
{
    ast_id left = ast->nodes[n].base.left;
    ast_id right = ast->nodes[n].base.right;

    if (depth >= ast_count_unsafe(ast))
        return 1;

    if (left > -1)
        if (max_depth_is_unreasonable(ast, left, depth + 1))
            return 1;
    if (right > -1)
        if (max_depth_is_unreasonable(ast, right, depth + 1))
            return 1;

    return 0;
}

int
ast_export_fp(
    const struct ast*      ast,
    FILE*                  fp,
    struct db_source       source,
    const struct cmd_list* cmds)
{
    struct utf8_list* cmd_names;
    int               n;
    char              magic[4] = {'A', 'S', 'T', '0'};

    if (ast == NULL)
        return 0;

    utf8_list_init(&cmd_names);

    if (fwrite(magic, sizeof(magic), 1, fp) != 1)
        goto error;

    if (fwrite(ast, offsetof(struct ast, nodes), 1, fp) != 1)
        goto error;
    for (n = 0; n != ast->count; ++n)
    {
        union ast_node node = ast->nodes[n];

        /* We don't want to export the original command list. */
        if (node.info.node_type == AST_COMMAND)
        {
            int              i;
            struct utf8_view name, compare;
            name = utf8_list_view(cmds->db_cmd_names, node.cmd.id);
            utf8_enumerate(cmd_names, i, compare)
            {
                if (utf8_equal(name, compare))
                {
                    node.cmd.id = i;
                    break;
                }
            }
            if (i == utf8_list_count(cmd_names))
            {
                node.cmd.id = utf8_list_count(cmd_names);
                if (utf8_list_add(&cmd_names, name) != 0)
                    goto error;
            }
        }

        if (fwrite(&node, sizeof(node), 1, fp) != 1)
            goto error;
    }

    if (cmd_names == NULL)
    {
        mem_size nothing = 0;
        if (fwrite(&nothing, sizeof(nothing), 1, fp) != 1)
            goto error;
    }
    else
    {
        mem_size header_size = offsetof(struct utf8_list, data);
        mem_size bytes = header_size + cmd_names->capacity;
        if (fwrite(&bytes, sizeof(bytes), 1, fp) != 1)
            goto error;
        if (fwrite(cmd_names, bytes, 1, fp) != 1)
            goto error;
    }

    if (fwrite(&source.text.len, sizeof(source.text.len), 1, fp) != 1)
        goto error;
    if (fwrite(source.text.data, source.text.len, 1, fp) != 1)
        goto error;

    if (fprintf(fp, "%c%c%c%c", magic[3], magic[2], magic[1], magic[0]) < 0)
        goto error;

    utf8_list_deinit(cmd_names);
    return 0;

error:
    utf8_list_deinit(cmd_names);
    return -1;
}

int
ast_export(
    const struct ast*      ast,
    struct ospathc         filepath,
    struct db_source       source,
    const struct cmd_list* cmds)
{
    FILE* fp;
    if (ast == NULL)
        return 0;

    fp = fopen(ospathc_cstr(filepath), "w");
    if (fp == NULL)
    {
        return log_err(
            "Failed to open file {quote:%s}: {emph:%s}\n",
            ospathc_cstr(filepath),
            strerror(errno));
    }

    if (ast_export_fp(ast, fp, source, cmds) != 0)
    {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}
