#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include <assert.h>

struct ptr_set_kvs
{
    const struct semantic_check** keys;
};
static hash32
ptr_kvs_hash(const struct semantic_check* key)
{
    return hash32_aligned_ptr((uintptr_t)key);
}
static int
ptr_kvs_alloc(
    struct ptr_set_kvs* kvs, struct ptr_set_kvs* old_kvs, int16_t capacity)
{
    kvs->keys = mem_alloc(sizeof(*kvs->keys) * capacity);
    return kvs->keys == NULL ? -1 : 0;
}
static void
ptr_kvs_free_old(struct ptr_set_kvs* kvs)
{
    mem_free(kvs->keys);
}
static void
ptr_kvs_free(struct ptr_set_kvs* kvs)
{
    mem_free(kvs->keys);
}
static const struct semantic_check*
ptr_kvs_get_key(const struct ptr_set_kvs* kvs, int16_t slot)
{
    return kvs->keys[slot];
}
static int
ptr_kvs_set_key(
    struct ptr_set_kvs* kvs, int16_t slot, const struct semantic_check* key)
{
    kvs->keys[slot] = key;
    return 0;
}
static int
ptr_kvs_keys_equal(
    const struct semantic_check* k1, const struct semantic_check* k2)
{
    return k1 == k2;
}
static void**
ptr_kvs_get_value(const struct ptr_set_kvs* kvs, int16_t slot)
{
    return (void**)1; // So insert_new() returns success
}
static void
ptr_kvs_set_value(struct ptr_set_kvs* kvs, int16_t slot, void** value)
{
}

HM_DECLARE_API_FULL(
    static,
    ptr_set,
    hash32,
    const struct semantic_check*,
    void*,
    16,
    struct ptr_set_kvs)
HM_DEFINE_API_FULL(
    ptr_set,
    hash32,
    const struct semantic_check*,
    void*,
    16,
    ptr_kvs_hash,
    ptr_kvs_alloc,
    ptr_kvs_free_old,
    ptr_kvs_free,
    ptr_kvs_get_key,
    ptr_kvs_set_key,
    ptr_kvs_keys_equal,
    ptr_kvs_get_value,
    ptr_kvs_set_value,
    32,
    70)

struct ctx
{
    struct ast**              tus;
    int                       tu_count;
    int                       tu_id;
    struct mutex**            tu_mutexes;
    const struct ospath*      filenames;
    struct utf8*              sources;
    const struct plugin_list* plugins;
    const struct cmd_list*    cmds;
    const struct globals*     globals;
    const struct udt_storage* udts;
};

static int
run_dependencies(
    struct ctx*                   ctx,
    const struct semantic_check** dependencies,
    struct ptr_set**              visited);

static int
run_check(
    struct ctx*                  ctx,
    const struct semantic_check* check,
    struct ptr_set**             visited)
{
    if (run_dependencies(ctx, check->depends_on, visited) < 0)
        return -1;

    if (ptr_set_emplace_new(visited, check) != NULL)
    {
        int result = check->execute(
            ctx->tus,
            ctx->tu_count,
            ctx->tu_id,
            ctx->tu_mutexes,
            ctx->filenames,
            ctx->sources,
            ctx->plugins,
            ctx->cmds,
            ctx->udts,
            ctx->globals);

#if defined(ODBCOMPILER_AST_DUMP)
        {
            const struct ast* ast = ctx->tus[ctx->tu_id];
            struct ospathc    filename = ospathc(ctx->filenames[ctx->tu_id]);
            struct utf8_view  source = utf8_view(ctx->sources[ctx->tu_id]);
            ast_export_basename(ast, filename, source, ctx->cmds);
        }
#endif
#if defined(ODBCOMPILER_AST_SANITY_CHECK)
        if (result == 0)
            result = ast_sanity_check(
                ctx->tus[ctx->tu_id],
                utf8_cstr(ctx->sources[ctx->tu_id]),
                ctx->cmds);
#endif

        return result;
    }

    return 0;
}
static int
run_dependencies(
    struct ctx*                   ctx,
    const struct semantic_check** dependencies,
    struct ptr_set**              visited)
{
    const struct semantic_check** check;
    for (check = dependencies; *check != NULL; ++check)
        if (run_check(ctx, *check, visited) != 0)
            return -1;
    return 0;
}

int
semantic_check_run(
    const struct semantic_check* check,
    struct ast**                 tus,
    int                          tu_count,
    int                          tu_id,
    struct mutex**               tu_mutexes,
    const struct ospath*         filenames,
    struct utf8*                 sources,
    const struct plugin_list*    plugins,
    const struct cmd_list*       cmds,
    const struct udt_storage*    udts,
    const struct globals*        globals)
{
    struct ptr_set* check_visited;
    struct ast**    astp = &tus[tu_id];
    struct ospathc  filename = ospathc(filenames[tu_id]);
    struct ctx      ctx
        = {tus,
           tu_count,
           tu_id,
           tu_mutexes,
           filenames,
           sources,
           plugins,
           cmds,
           globals,
           udts};

    if (ast_count(*astp) == 0)
    {
        log_warn(
            "AST is empty for source file {quote:%s}\n",
            ospathc_cstr(filename));
        return 0;
    }

    ptr_set_init(&check_visited);

    if (run_check(&ctx, check, &check_visited) < 0)
    {
        ptr_set_deinit(check_visited);
        return -1;
    }

    ptr_set_deinit(check_visited);

    return 0;
}

static int
dummy_check(
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
    return 0;
}

int
semantic_run_essential_checks(
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
    static const struct semantic_check* essential_checks[]
        = {&semantic_type_check,
           &semantic_resolve_command_overloads,
           &semantic_loop_exit,
           &semantic_loop_cont,
           &semantic_loop_for,
           &semantic_select,
           NULL};
    static const struct semantic_check essential_check
        = {dummy_check, essential_checks, "essential_checks"};

    return semantic_check_run(
        &essential_check,
        tus,
        tu_count,
        tu_id,
        tu_mutexes,
        filenames,
        sources,
        plugins,
        cmds,
        udts,
        globals);
}
