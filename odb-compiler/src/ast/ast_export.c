#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include <errno.h>
#include <stdio.h>

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
ast_export_filename(
    const struct ast*      ast,
    const char*            filename,
    struct db_source       source,
    const struct cmd_list* cmds)
{
    int           result;
    struct ospath fname = empty_ospath();

    if (ast == NULL)
        return 0;

    ospath_set_cstr(&fname, filename);
    ospath_filename(&fname);
    utf8_append_cstr(&fname.str, ".ast");
    result = ast_export(ast, ospathc(fname), source, cmds);
    ospath_deinit(fname);
    return result;
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
