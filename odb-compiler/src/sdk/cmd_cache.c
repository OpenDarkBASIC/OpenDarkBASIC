#include "odb-compiler/sdk/cmd_cache.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/utf8.h"

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
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform)
{
    struct utf8   fname = empty_utf8();
    struct ospath path = empty_ospath();

    if (fs_get_appdata_dir(&path) != 0)
        goto error;

    if (utf8_fmt(
            &fname,
            "%s-%s-%s.dat",
            sdk_type_to_name(sdk_type),
            target_arch_to_name(arch),
            target_platform_to_name(platform))
        != 0)
    {
        goto error;
    }

    if (ospath_join_cstr(&path, "cmd-cache") != 0)
        goto error;
    if (ospath_join_cstr(&path, utf8_cstr(fname)) != 0)
        goto error;

    return 0;

error:
    ospath_deinit(path);
    utf8_deinit(fname);
    return -1;
}

int
cmd_cache_save(
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform)
{
    struct utf8   fname = empty_utf8();
    struct ospath path = empty_ospath();

    if (fs_get_appdata_dir(&path) != 0)
        goto error;

    if (utf8_fmt(
            &fname,
            "%s-%s-%s.dat",
            sdk_type_to_name(sdk_type),
            target_arch_to_name(arch),
            target_platform_to_name(platform))
        != 0)
    {
        goto error;
    }

    if (ospath_join_cstr(&path, "cmd-cache") != 0)
        goto error;
    if (ospath_join_cstr(&path, utf8_cstr(fname)) != 0)
        goto error;

    return 0;

error:
    ospath_deinit(path);
    utf8_deinit(fname);
    return -1;
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
