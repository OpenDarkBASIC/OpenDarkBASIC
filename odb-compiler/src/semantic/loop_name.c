#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/semantic/semantic.h"

static int
find_parent_loop_with_same_name(
    const struct ast* ast,
    ast_id            loop,
    struct ospathc    filename,
    const char*       source)
{
    struct utf8_span ename = ast->nodes[loop].loop1.name;
    struct utf8_span iname = ast->nodes[loop].loop1.implicit_name;
    while ((loop = ast_find_parent(ast, loop)) > -1)
    {
        /* Can't cross function boundaries */
        if (ast_node_type(ast, loop) == AST_FUNC1)
            break;

        if (ast_node_type(ast, loop) == AST_LOOP1)
        {
            struct utf8_span outer_ename = ast->nodes[loop].loop1.name;
            struct utf8_span outer_iname = ast->nodes[loop].loop1.implicit_name;

            if (iname.len && outer_ename.len
                && utf8_equal_span(source, iname, outer_ename))
                return err_loop_duplicate_name(
                    ast, iname, outer_ename, filename, source);
            if (ename.len && outer_iname.len
                && utf8_equal_span(source, ename, outer_iname))
                return err_loop_duplicate_name(
                    ast, ename, outer_iname, filename, source);
            if (ename.len && outer_ename.len
                && utf8_equal_span(source, ename, outer_ename))
                return err_loop_duplicate_name(
                    ast, ename, outer_ename, filename, source);
        }
    }

    return 0;
}

static int
check_loop_names(
    struct ast**              tus,
    int                       tu_count,
    int                       tu_id,
    struct mutex**            tu_mutexes,
    const struct ospath*      filenames,
    struct utf8*              sources,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const struct udt_storage* udts,
    const struct globals*     globals)
{
    ast_id         n;
    struct ast*    ast = tus[tu_id];
    struct ospathc filename = ospathc(filenames[tu_id]);
    const char*    source = sources[tu_id].data;
    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_LOOP1)
            continue;

        if (find_parent_loop_with_same_name(ast, n, filename, source) != 0)
            return -1;
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_name
    = {check_loop_names, depends, "loop_name"};
