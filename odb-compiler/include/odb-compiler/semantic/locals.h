#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-compiler/config.h"
#include "odb-util/hm.h"

struct hm;

/* Used to track the existence and types of variables for the
 * current scope.
 *
 * When a variable, function, or UDT first appears, its name is inserted into
 * the list of locals along with the necessary type information. If the
 * variable, function or UDT is later referenced, then the type is extracted
 * from the list again.
 *
 * @note The names, locations and node indices stored in the list all reference
 * an external AST. The lifetime of this structure must therefore be shorter
 * than that of the AST, filename and source code. */
struct locals
{
    struct hm* name_map;
};

/* An entry in the local symbol table */
struct local
{
    /* Points to the identifier that first created the entry. It needs to be a
     * ast_id rather than a utf8_span, because some checks rely on checking if
     * they created the entry or not and the same names can have different
     * spans. */
    ast_id first_occurrence;

    /* The parent node that created the entry. When a type is not resolvable,
     * the stack is popped up until this node. */
    ast_id dependent;

    union type type;
};

ODBCOMPILER_PUBLIC_API void
locals_init(struct locals* locals);

ODBCOMPILER_PUBLIC_API void
locals_deinit(struct locals* locals);

static inline void
local_init(
    struct local* local,
    ast_id        first_occurrence,
    ast_id        dependent,
    union type    type)
{
    local->first_occurrence = first_occurrence;
    local->dependent = dependent;
    local->type = type;
}

ODBCOMPILER_PUBLIC_API enum hm_status
locals_declare(
    struct locals*   locals,
    struct utf8_span identifier_name,
    int32_t          scope_id,
    const char*      source,
    struct local**   value);

ODBCOMPILER_PUBLIC_API enum hm_status
locals_find_or_declare(
    struct locals*   locals,
    struct utf8_span identifier_name,
    int32_t          scope_id,
    const char*      source,
    struct local**   value);

ODBCOMPILER_PUBLIC_API struct local*
locals_find(
    const struct locals* locals,
    struct utf8_span     identifier_name,
    int32_t              scope_id,
    const char*          source);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_locals(struct locals* locals);
ODBCOMPILER_PUBLIC_API void
mem_release_locals(struct locals* locals);
#else
#define mem_acquire_(locals)
#define mem_release_locals(locals)
#endif
