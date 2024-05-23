#include "odb-compiler/sdk/cmd_cache.h"

void
cmd_cache_init(struct cmd_cache* cache)
{
}

void
cmd_cache_deinit(struct cmd_cache* cache)
{
}

int
cmd_cache_load(struct cmd_cache* cache)
{
    return 0;
}

int
cmd_cache_save(struct cmd_cache* cache)
{
    return 0;
}

int
cmd_cache_plugin_needs_reload(
    struct cmd_cache* cache, const struct plugin_info* plugin)
{
    return 1;
}

int
cmd_cache_add_plugin(struct cmd_cache* cache, const struct plugin_info* plugin)
{
    return 0;
}
