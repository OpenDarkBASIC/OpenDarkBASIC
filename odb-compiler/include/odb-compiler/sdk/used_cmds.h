#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/cmd_list.h"

VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, cmd_ids, cmd_id, 32)

struct ast;
struct used_cmds;

ODBCOMPILER_PUBLIC_API void
used_cmds_init(struct used_cmds** hm);

ODBCOMPILER_PUBLIC_API int
used_cmds_append(struct used_cmds** used, const struct ast* ast);

ODBCOMPILER_PUBLIC_API struct cmd_ids*
used_cmds_finalize(struct used_cmds* hm);
