#include "odb-compiler/sdk/cmd_cache.h"
#include "odb-sdk/fs.h"

void
cmd_cache_init(struct cmd_cache* cache)
{
}

void
cmd_cache_deinit(struct cmd_cache* cache)
{
}

int
cmd_cache_load(
    struct cmd_cache*    cache,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform)
{
    struct ospath path = empty_ospath();
    if (fs_get_appdata_dir(&path) != 0)
        return -1;

    return 0;
}

int
cmd_cache_save(
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform)
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
