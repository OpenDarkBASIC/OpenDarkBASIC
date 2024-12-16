#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/sdk_type.h"

struct plugin_ids;
struct plugin_list;
struct udt_storage;

int
cmd_cache_load(
    struct plugin_ids**       cached_plugins,
    const struct plugin_list* plugins,
    struct cmd_list*          cmds,
    struct udt_storage*       udts,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);
int
cmd_cache_save(
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const struct udt_storage* udts,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);

ODBCOMPILER_PUBLIC_API int
cmd_cache_delete(
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform);
