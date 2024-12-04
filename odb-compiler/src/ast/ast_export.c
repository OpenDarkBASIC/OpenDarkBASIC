#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/log.h"
#include <errno.h>
#include <stdio.h>

int
ast_export_fp(
    const struct ast*      ast,
    FILE*                  fp,
    struct utf8_view       source,
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
            name = utf8_list_view(cmds->cmd_names, node.command.id);
            utf8_enumerate(cmd_names, i, compare)
            {
                if (utf8_equal(name, compare))
                {
                    node.command.id = i;
                    break;
                }
            }
            if (i == utf8_list_count(cmd_names))
            {
                node.command.id = utf8_list_count(cmd_names);
                if (utf8_list_add(&cmd_names, name) != 0)
                    goto error;
            }
        }

        if (fwrite(&node, sizeof(node), 1, fp) != 1)
            goto error;
    }

    if (cmd_names == NULL)
    {
        utf8_idx nothing = 0;
        if (fwrite(&nothing, sizeof(nothing), 1, fp) != 1)
            goto error;
    }
    else
    {
        /* list has utf8_span structures stored at the end of the buffer, so we
         * have to write the capacity, not the count */
        utf8_idx header_size = offsetof(struct utf8_list, data);
        utf8_idx bytes = header_size + cmd_names->capacity;
        if (fwrite(&bytes, sizeof(bytes), 1, fp) != 1)
            goto error;
        if (fwrite(cmd_names, bytes, 1, fp) != 1)
            goto error;
    }

    if (fwrite(&source.len, sizeof(source.len), 1, fp) != 1)
        goto error;
    if (fwrite(source.data, source.len, 1, fp) != 1)
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
ast_export_basename(
    const struct ast*      ast,
    struct ospathc         filename,
    struct utf8_view       source,
    const struct cmd_list* cmds)
{
    int           result;
    struct ospath fname = empty_ospath();

    if (ast == NULL)
        return 0;

    ospath_set(&fname, filename);
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
    struct utf8_view       source,
    const struct cmd_list* cmds)
{
    FILE*        fp;
    struct utf16 utf16 = empty_utf16();

    if (ast == NULL)
        return 0;

#if defined(ODBCOMPILER_PLATFORM_WINDOWS)
    if (utf8_to_utf16(&utf16, ospathc_view(filepath)) != 0)
        return -1;
    fp = _wfopen(utf16_cstr(utf16), L"wb");
    utf16_deinit(utf16);
#else
    fp = fopen(ospathc_cstr(filepath), "wb");
    (void)utf16;
#endif
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
