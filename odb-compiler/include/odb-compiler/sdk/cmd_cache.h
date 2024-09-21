#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/sdk_type.h"

struct plugin_ids;
struct plugin_list;

int
cmd_cache_load(
    struct plugin_ids**       cached_plugins,
    const struct plugin_list* plugins,
    struct cmd_list*          cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);
int
cmd_cache_save(
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);
