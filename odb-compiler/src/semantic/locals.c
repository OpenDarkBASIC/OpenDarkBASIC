#include "odb-compiler/semantic/locals.h"
#include "odb-util/utf8.h"

struct span_scope
{
    struct utf8_span span;
    int16_t          scope;
};

struct view_scope
{
    struct utf8_view view;
    int16_t          scope;
};

VEC_DECLARE_API(static, span_scopes, struct span_scope, 32)
VEC_DEFINE_API(span_scopes, struct span_scope, 32)

/* The "text" field references the source text. The span_scopes contains
 * utf8_span's that index into the source code. Because it's possible to have
 * the same variable name in a different scope, the key also contains the
 * current scope (0=global, 1, 2, 3, ... = nesting) such that the same variable
 * name hashes to a different value if it is in a different scope. */
struct hm_kvs
{
    const char*         text;
    struct span_scopes* keys;
    struct local*       values;
};

static hash32
hm_kvs_hash(struct view_scope key)
{
    return hash32_jenkins_oaat(key.view.data + key.view.off, key.view.len)
           + key.scope;
}
static int
hm_kvs_alloc(struct hm_kvs* kvs, struct hm_kvs* old_kvs, int32_t capacity)
{
    kvs->text = NULL;
    span_scopes_init(&kvs->keys);
    if (span_scopes_resize(&kvs->keys, capacity) != 0)
        return -1;

    if ((kvs->values = mem_alloc(sizeof(*kvs->values) * capacity)) == NULL)
    {
        span_scopes_deinit(kvs->keys);
        return log_oom(sizeof(*kvs->values) * capacity, "hm_kvs_alloc()");
    }

    return 0;
}
static void
hm_kvs_free_old(struct hm_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
}
static void
hm_kvs_free(struct hm_kvs* kvs)
{
    mem_free(kvs->values);
    span_scopes_deinit(kvs->keys);
}
static struct view_scope
hm_kvs_get_key(const struct hm_kvs* kvs, int32_t slot)
{
    ODBUTIL_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
hm_kvs_set_key(struct hm_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBUTIL_DEBUG_ASSERT(
        kvs->text == NULL || kvs->text == key.view.data, (void)0);

    kvs->text = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->text, key.view);
    struct span_scope span_scope = {span, key.scope};
    kvs->keys->data[slot] = span_scope;

    return 0;
}
static int
hm_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static struct local*
hm_kvs_get_value(const struct hm_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
hm_kvs_set_value(struct hm_kvs* kvs, int32_t slot, struct local* value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    static, hm, hash32, struct view_scope, struct local, 32, struct hm_kvs)
HM_DEFINE_API_FULL(
    hm,
    hash32,
    struct view_scope,
    struct local,
    32,
    hm_kvs_hash,
    hm_kvs_alloc,
    hm_kvs_free_old,
    hm_kvs_free,
    hm_kvs_get_key,
    hm_kvs_set_key,
    hm_kvs_keys_equal,
    hm_kvs_get_value,
    hm_kvs_set_value,
    32,
    70)

enum hm_status
locals_declare(
    struct locals*   locals,
    struct utf8_span identifier_name,
    int32_t          scope_id,
    const char*      source,
    struct local**   value)
{
    struct view_scope key = {utf8_span_view(source, identifier_name), scope_id};
    return hm_emplace_or_get(&locals->name_map, key, value);
}

enum hm_status
locals_find_or_declare(
    struct locals*   locals,
    struct utf8_span identifier_name,
    int32_t          scope_id,
    const char*      source,
    struct local**   value)
{
    struct view_scope key = {utf8_span_view(source, identifier_name), scope_id};
    /* TODO: scope_id needs to also contain the parent scope so we can access
     * global variables and outer scopes */
    *value = hm_find(locals->name_map, key);
    if (*value == NULL)
    {
        key.scope = 0; /* XXX: Global scope */
        return hm_emplace_or_get(&locals->name_map, key, value);
    }
    return HM_EXISTS;
}

struct local*
locals_find(
    const struct locals* locals,
    struct utf8_span     identifier_name,
    int32_t              scope_id,
    const char*          source)
{
    struct view_scope key = {utf8_span_view(source, identifier_name), scope_id};
    struct local*     value = hm_find(locals->name_map, key);
    if (value == NULL)
    {
        /* TODO: scope_id needs to also contain the parent scope so we can
         * access global variables and outer scopes */
        key.scope = 0; /* XXX: Global scope */
        value = hm_find(locals->name_map, key);
    }
    return value;
}
