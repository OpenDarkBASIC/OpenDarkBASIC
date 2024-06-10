#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/sdk_type.h"

struct ast;
struct cmd_list;
struct ir_module;

ODBCOMPILER_PUBLIC_API struct ir_module*
ir_alloc(const char* module_name);

ODBCOMPILER_PUBLIC_API void
ir_free(struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_translate_ast(
    struct ir_module*      ir,
    struct ast*            program,
    enum sdk_type          sdkType,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source);

ODBCOMPILER_PUBLIC_API int
ir_create_runtime(
    struct ir_module*    ir,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform);

ODBCOMPILER_PUBLIC_API int
ir_optimize(struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_compile(
    struct ir_module*    mod,
    const char*          output_filepath,
    enum target_arch     arch,
    enum target_platform platform);
