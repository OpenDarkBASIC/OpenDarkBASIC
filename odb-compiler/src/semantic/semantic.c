#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include <assert.h>

struct ptr_kvs
{
    const struct semantic_check** keys;
};
static hash32
ptr_kvs_hash(const struct semantic_check* key)
{
    return hash32_aligned_ptr((uintptr_t)key);
}
static int
ptr_kvs_alloc(struct ptr_kvs* kvs, struct ptr_kvs* old_kvs, int16_t capacity)
{
    kvs->keys = mem_alloc(sizeof(*kvs->keys) * capacity);
    return kvs->keys == NULL ? -1 : 0;
}
static void
ptr_kvs_free(struct ptr_kvs* kvs)
{
    mem_free(kvs->keys);
}
static const struct semantic_check*
ptr_kvs_get_key(const struct ptr_kvs* kvs, int16_t slot)
{
    return kvs->keys[slot];
}
static int
ptr_kvs_set_key(
    struct ptr_kvs* kvs, int16_t slot, const struct semantic_check* key)
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
static void*
ptr_kvs_get_value(const struct ptr_kvs* kvs, int16_t slot)
{
    return (void*)1; // So insert_new() returns success
}
static void
ptr_kvs_set_value(struct ptr_kvs* kvs, int16_t slot, void* value)
{
}

HM_DECLARE_API_FULL(
    static,
    ptr_set,
    hash32,
    const struct semantic_check*,
    void*,
    16,
    struct ptr_kvs)
HM_DEFINE_API_FULL(
    ptr_set,
    hash32,
    const struct semantic_check*,
    void*,
    16,
    ptr_kvs_hash,
    ptr_kvs_alloc,
    ptr_kvs_free,
    ptr_kvs_get_key,
    ptr_kvs_set_key,
    ptr_kvs_keys_equal,
    ptr_kvs_get_value,
    ptr_kvs_set_value,
    32,
    70)

static int
run_dependencies(
    const struct semantic_check** dependencies,
    struct ptr_set**              visited,
    struct ast*                   ast,
    const struct plugin_list*     plugins,
    const struct cmd_list*        cmds,
    const struct symbol_table*    symbols,
    const char*                   source_filename,
    const char*                   source_text);

static int
run_check(
    const struct semantic_check* check,
    struct ptr_set**             visited,
    struct ast*                  ast,
    const struct plugin_list*    plugins,
    const struct cmd_list*       cmds,
    const struct symbol_table*   symbols,
    const char*                  source_filename,
    const char*                  source_text)
{
    if (run_dependencies(
            check->depends_on,
            visited,
            ast,
            plugins,
            cmds,
            symbols,
            source_filename,
            source_text)
        < 0)
        return -1;

    if (ptr_set_emplace_new(visited, check))
        if (check->execute(
                ast, plugins, cmds, symbols, source_filename, source_text)
            < 0)
        {
            return -1;
        }

    return 0;
}
static int
run_dependencies(
    const struct semantic_check** dependencies,
    struct ptr_set**              visited,
    struct ast*                   ast,
    const struct plugin_list*     plugins,
    const struct cmd_list*        cmds,
    const struct symbol_table*    symbols,
    const char*                   source_filename,
    const char*                   source_text)
{
    const struct semantic_check** check;
    for (check = dependencies; *check != NULL; ++check)
        if (run_check(
                *check,
                visited,
                ast,
                plugins,
                cmds,
                symbols,
                source_filename,
                source_text)
            != 0)
            return -1;
    return 0;
}

int
semantic_check_run(
    const struct semantic_check* check,
    struct ast*                  ast,
    const struct plugin_list*    plugins,
    const struct cmd_list*       cmds,
    const struct symbol_table*   symbols,
    const char*                  source_filename,
    const char*                  source_text)
{
    struct ptr_set* check_visited;
    if (ast->node_count == 0)
    {
        log_semantic_warn(
            "AST is empty for source file {quote:%s}\n", source_filename);
        return 0;
    }

    ptr_set_init(&check_visited);

    if (run_check(
            check,
            &check_visited,
            ast,
            plugins,
            cmds,
            symbols,
            source_filename,
            source_text)
        < 0)
    {
        ptr_set_deinit(check_visited);
        return -1;
    }

    ptr_set_deinit(check_visited);
    return 0;
}

static int
dummy_check(
    struct ast*                ast,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols,
    const char*                source_filename,
    const char*                source_text)
{
    return 0;
}

int
semantic_run_essential_checks(
    struct ast*                ast,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols,
    const char*                source_filename,
    const char*                source_text)
{
    static const struct semantic_check* essential_checks[]
        = {&semantic_expand_constant_declarations,
           &semantic_type_check,
           &semantic_resolve_cmd_overloads,
           &semantic_loop_exit,
           &semantic_loop_cont,
           &semantic_loop_for,
           NULL};
    static const struct semantic_check essential_check
        = {dummy_check, essential_checks};

    return semantic_check_run(
        &essential_check,
        ast,
        plugins,
        cmds,
        symbols,
        source_filename,
        source_text);
}
