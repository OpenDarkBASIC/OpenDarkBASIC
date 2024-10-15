#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/config.h"
#include "odb-compiler/sdk/sdk_type.h"

struct ast;
struct cmd_ids;
struct cmd_list;
struct ir_module;

ODBCOMPILER_PUBLIC_API struct ir_module*
ir_alloc(const char* module_name);

ODBCOMPILER_PUBLIC_API void
ir_free(struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_translate_ast(
    struct ir_module*      ir,
    const struct ast*      ast,
    enum sdk_type          sdkType,
    enum target_arch       arch,
    enum target_platform   platform,
    const struct cmd_list* cmds,
    const char*            source_filename,
    const char*            source_text);

ODBCOMPILER_PUBLIC_API int
ir_create_harness(
    struct ir_module*         ir,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const struct cmd_ids*     used_cmds,
    const char*               main_dba_name,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);

ODBCOMPILER_PUBLIC_API int
ir_optimize(struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_dump(const struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_compile(
    struct ir_module*    mod,
    const char*          output_filepath,
    enum target_arch     arch,
    enum target_platform platform);
