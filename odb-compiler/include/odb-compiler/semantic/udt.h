#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/ospath.h"
#include "odb-util/utf8.h"

struct udt_storage
{
    struct ast* ast;
    struct utf8 source;
};

ODBCOMPILER_PUBLIC_API void
udt_storage_init(struct udt_storage* udts);

ODBCOMPILER_PUBLIC_API void
udt_storage_deinit(struct udt_storage* udts);

ODBCOMPILER_PUBLIC_API union type
udt_storage_add_type(
    struct udt_storage* udts,
    struct utf8_span    type_name,
    const struct ast*   ast,
    struct ospathc      filename,
    const char*         source);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_udt_storage(struct udt_storage* udts);
ODBCOMPILER_PUBLIC_API void
mem_release_udt_storage(struct udt_storage* udts);
#else
#define mem_acquire_udt_storage(udts)
#define mem_release_udt_storage(udts)
#endif
