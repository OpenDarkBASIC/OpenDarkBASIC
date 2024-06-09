#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/sdk/sdk.h"

struct plugin_info;
struct plugin_list;

struct cmd_cache
{
    char dummy;
};

void
cmd_cache_init(struct cmd_cache* cache);
void
cmd_cache_deinit(struct cmd_cache* cache);

int
cmd_cache_load(
    struct cmd_cache*    cache,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform);
int
cmd_cache_save(
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform);

int
cmd_cache_plugin_needs_reload(
    struct cmd_cache* cache, const struct plugin_info* plugin);

int
cmd_cache_add_plugin(struct cmd_cache* cache, const struct plugin_info* plugin);
