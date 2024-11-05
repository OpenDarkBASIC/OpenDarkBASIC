#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/globals.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/mem.h"

struct kvs_key_data
{
    utf8_idx count, capacity;
    char     data[1];
};

struct hm_kvs
{
    struct utf8_span*    key_spans;
    struct kvs_key_data* key_data;
    struct global*       values;
};

static hash32
kvs_hash(struct utf8_view key)
{
    // TODO: Could just use the offset/length here?
    return hash32_jenkins_oaat(key.data + key.off, key.len);
}

static int
kvs_alloc(struct hm_kvs* kvs, struct hm_kvs* old_kvs, int32_t capacity)
{
    static const int avg_func_name_len = 32;
    int              header_size = offsetof(struct kvs_key_data, data);
    int              data_size = sizeof(char) * avg_func_name_len * capacity;
    kvs->key_data
        = old_kvs ? old_kvs->key_data : mem_alloc(header_size + data_size);
    if (kvs->key_data == NULL)
        goto alloc_data_failed;
    if (old_kvs == NULL)
        kvs->key_data->count = 0;
    kvs->key_data->capacity = capacity;

    kvs->key_spans = mem_alloc(sizeof(*kvs->key_spans) * capacity);
    if (kvs->key_spans == NULL)
        goto alloc_spans_failed;

    kvs->values = mem_alloc(sizeof(*kvs->values) * capacity);
    if (kvs->values == NULL)
        goto alloc_values_failed;

    return 0;

alloc_values_failed:
    mem_free(kvs->key_spans);
alloc_spans_failed:
    if (old_kvs == NULL)
        mem_free(kvs->key_data);
alloc_data_failed:
    return -1;
}

static void
kvs_free_old(struct hm_kvs* kvs)
{
    /* kvs->key_data ownership moved to the new kvs */
    mem_free(kvs->key_spans);
    mem_free(kvs->values);
}

static void
kvs_free(struct hm_kvs* kvs)
{
    mem_free(kvs->key_data);
    mem_free(kvs->key_spans);
    mem_free(kvs->values);
}

static struct utf8_view
kvs_get_key(const struct hm_kvs* kvs, utf8_idx idx)
{
    return utf8_span_view(kvs->key_data->data, kvs->key_spans[idx]);
}

static int
kvs_set_key(struct hm_kvs* kvs, utf8_idx idx, struct utf8_view key)
{
    while (kvs->key_data->count + key.len > kvs->key_data->capacity)
    {
        int   header_size = offsetof(struct kvs_key_data, data);
        int   data_size = sizeof(char) * kvs->key_data->capacity * 2;
        void* new_mem = mem_realloc(kvs->key_data, header_size + data_size);
        if (new_mem == NULL)
            return log_oom(header_size + data_size, "kvs_set_key()");
        kvs->key_data = new_mem;
        kvs->key_data->capacity *= 2;
    }

    kvs->key_spans[idx].off = kvs->key_data->count;
    kvs->key_spans[idx].len = key.len;
    memcpy(
        kvs->key_data->data + kvs->key_data->count,
        key.data + key.off,
        key.len);
    kvs->key_data->count += key.len;

    return 0;
}

static int
kvs_keys_equal(struct utf8_view a, struct utf8_view b)
{
    return utf8_equal(a, b);
}

static struct global*
kvs_get_value(const struct hm_kvs* kvs, utf8_idx idx)
{
    return &kvs->values[idx];
}

static void
kvs_set_value(struct hm_kvs* kvs, utf8_idx idx, struct global* value)
{
    kvs->values[idx] = *value;
}

HM_DECLARE_API_FULL(
    static, hm, hash32, struct utf8_view, struct global, 32, struct hm_kvs)
HM_DEFINE_API_FULL(
    hm,
    hash32,
    struct utf8_view,
    struct global,
    32,
    kvs_hash,
    kvs_alloc,
    kvs_free_old,
    kvs_free,
    kvs_get_key,
    kvs_set_key,
    kvs_keys_equal,
    kvs_get_value,
    kvs_set_value,
    128,
    70)
struct globals
{
    struct hm hm;
};

void
globals_deinit(struct globals* globals)
{
    hm_deinit(&globals->hm);
}

static int
add_function(
    struct globals**        globals,
    struct ast**            tus,
    int                     tu_id,
    ast_id                  f1,
    const struct utf8*      filenames,
    const struct db_source* sources)
{
    struct global*    entry;
    ast_id            identifier;
    struct utf8_span  func_span;
    struct utf8_view  func_name;
    const struct ast* ast = tus[tu_id];
    const char*       source = sources[tu_id].text.data;

    identifier = ast->nodes[f1].func1.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    func_span = ast->nodes[identifier].identifier.name;
    func_name = utf8_span_view(source, func_span);

    switch (hm_emplace_or_get((struct hm**)globals, func_name, &entry))
    {
        case HM_OOM: return -1;
        case HM_NEW: {
            /* If the function is polymorphic, we want to store the
             * polymorphic function node. It's easier to instantiate the
             * functions in type_check.c this way */
            ast_id parent = ast_find_parent(ast, f1);
            if (parent > -1 && ast_node_type(ast, parent) == AST_FUNC_POLY)
                f1 = parent;

            entry->tu_id = tu_id;
            entry->ast_node = f1;
            break;
        }

        case HM_EXISTS: {
            const struct ast* prev_ast = tus[entry->tu_id];
            ast_id            prev_f1
                = ast_node_type(prev_ast, entry->ast_node) == AST_FUNC_POLY
                      ? prev_ast->nodes[entry->ast_node].func_poly.func
                      : entry->ast_node;
            const char* prev_filename = utf8_cstr(filenames[entry->tu_id]);
            const char* prev_source = sources[entry->tu_id].text.data;
            const char* filename = utf8_cstr(filenames[tu_id]);
            return err_func_redefinition(
                ast,
                f1,
                filename,
                source,
                prev_ast,
                prev_f1,
                prev_filename,
                prev_source);
        }
    }

    return 0;
}

static int
add_udt_decl(
    struct globals**        globals,
    struct ast**            tus,
    int                     tu_id,
    ast_id                  udt_decl,
    const struct utf8*      filenames,
    const struct db_source* sources)
{
    struct global*    entry;
    struct utf8_span  udt_span;
    struct utf8_view  udt_name;
    const struct ast* ast = tus[tu_id];
    const char*       source = sources[tu_id].text.data;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(ast, udt_decl)));

    udt_span = ast->nodes[udt_decl].udt_decl.type_name;
    udt_name = utf8_span_view(source, udt_span);
    switch (hm_emplace_or_get((struct hm**)globals, udt_name, &entry))
    {
        case HM_OOM: return -1;
        case HM_NEW: {
            entry->tu_id = tu_id;
            entry->ast_node = udt_decl;
            break;
        }

        case HM_EXISTS: {
            const struct ast* prev_ast = tus[entry->tu_id];
            ast_id            prev_udt_decl = entry->ast_node;
            const char* prev_filename = utf8_cstr(filenames[entry->tu_id]);
            const char* prev_source = sources[entry->tu_id].text.data;
            const char* filename = utf8_cstr(filenames[tu_id]);
            return err_udt_decl_redeclaration(
                ast,
                udt_span,
                filename,
                source,
                prev_ast,
                ast->nodes[prev_udt_decl].udt_decl.type_name,
                prev_filename,
                prev_source);
        }
    }

    return 0;
}

int
globals_add_declarations_from_ast(
    struct globals**        table,
    struct ast**            tus,
    int                     tu_id,
    const struct utf8*      filenames,
    const struct db_source* sources)
{
    ast_id n;
    for (n = 0; n != ast_count(tus[tu_id]); ++n)
    {
        if (ast_node_type(tus[tu_id], n) == AST_FUNC1)
            if (add_function(table, tus, tu_id, n, filenames, sources) != 0)
                return -1;

        if (ast_node_type(tus[tu_id], n) == AST_UDT_DECL)
            if (add_udt_decl(table, tus, tu_id, n, filenames, sources) != 0)
                return -1;
    }

    return 0;
}

const struct global*
globals_find(const struct globals* table, struct utf8_view key)
{
    return hm_find(&table->hm, key);
}

#if defined(ODBUTIL_MEM_DEBUGGING)
void
mem_acquire_globals(struct globals* table)
{
    if (table == NULL)
        return;

    ODBUTIL_DEBUG_ASSERT(table->hm.kvs.key_data != NULL, (void)0);
    ODBUTIL_DEBUG_ASSERT(table->hm.kvs.key_spans != NULL, (void)0);
    ODBUTIL_DEBUG_ASSERT(table->hm.kvs.values != NULL, (void)0);

    mem_acquire(
        table,
        offsetof(struct hm, hashes)
            + table->hm.capacity * sizeof(table->hm.hashes[0]));
    mem_acquire(
        table->hm.kvs.key_data,
        offsetof(struct kvs_key_data, data)
            + sizeof(table->hm.kvs.key_data->data[0])
                  * table->hm.kvs.key_data->capacity);
    mem_acquire(
        table->hm.kvs.key_spans,
        sizeof(table->hm.kvs.key_spans[0]) * table->hm.capacity);
    mem_acquire(
        table->hm.kvs.values,
        sizeof(table->hm.kvs.values[0]) * table->hm.capacity);
}
void
mem_release_globals(struct globals* table)
{
    if (table == NULL)
        return;

    mem_release(table->hm.kvs.values);
    mem_release(table->hm.kvs.key_spans);
    mem_release(table->hm.kvs.key_data);
    mem_release(table);
}
#endif
