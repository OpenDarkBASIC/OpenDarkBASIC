#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/udt.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"

void
udt_storage_init(struct udt_storage* udts)
{
    udts->source = empty_utf8();
    ast_init(&udts->ast);
}

void
udt_storage_deinit(struct udt_storage* udts)
{
    ast_deinit(udts->ast);
    utf8_deinit(udts->source);
}

union type
udt_storage_add_type(
    struct udt_storage* udts,
    struct utf8_span    type_name,
    const struct ast*   ast,
    struct ospathc      filename,
    const char*         source)
{
    ast_id n;
    for (n = 0; n != ast_count(ast); ++n)
    {
        ast_id           ident, udt_decl;
        struct utf8_span span;
        struct utf8_view name;

        if (ast_node_type(ast, n) != AST_UDT_DECL)
            continue;

        ident = ast->nodes[n].udt_decl.type_identifier;
        span = ast->nodes[ident].identifier.name;
        name = utf8_span_view(source, span);
        if (!utf8_equal(name, utf8_span_view(source, type_name)))
            continue;

        udt_decl
            = ast_dup_subtree_into(&udts->ast, &udts->source, ast, n, source);
        if (udt_decl < 0)
            return type_invalid();

        return type_udt(udt_decl);
    }

    log_flc(filename, source, type_name);
    log_err(
        "User-Defined Type '%.*s' not found\n",
        type_name.len,
        source + type_name.off);
    log_excerpt_1(source, type_name, empty_utf8_view(), 0);

    return type_invalid();
}

#if defined(ODBUTIL_MEM_DEBUGGING)
void
mem_acquire_udt_storage(struct udt_storage* udts)
{
    mem_acquire_ast(udts->ast);
    mem_acquire(udts->source.data, udts->source.len);
}

void
mem_release_udt_storage(struct udt_storage* udts)
{
    mem_release(udts->source.data);
    mem_release_ast(udts->ast);
}
#endif
