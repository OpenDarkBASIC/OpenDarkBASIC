#include "odb-compiler/sdk/plugin_info.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"

VEC_DEFINE_API(plugin_list, struct plugin_info, 16)

struct on_plugin_entry_ctx
{
    struct plugin_list* plugins;
    struct ospath_view dir;
};

static int
on_plugin_entry(const char* cname, void* user)
{
    struct on_plugin_entry_ctx* ctx = user;
    struct ospath_view name = cstr_ospath_view(cname);

    if (ospath_ends_with_i_cstr(name, ".dll")
        || ospath_ends_with_i_cstr(name, ".so"))
    {
        struct plugin_info* plugin = plugin_list_emplace(ctx->plugins);
        if (plugin == NULL)
            return -1;
        plugin_info_init(plugin);

        if (ospath_set(&plugin->filepath, ctx->dir) != 0)
            return -1;
        if (ospath_join(&plugin->filepath, name) != 0)
            return -1;

        if (utf8_set(&plugin->name, &plugin->name_range, name.str, name.range) != 0)
            return -1;
        /* Remove extension */
        while (plugin->name_range.len-- && plugin->name.data[plugin->name_range.off + plugin->name_range.len] != '.') {}
    }
    
    return 0;
}

int
plugin_list_populate(
    struct plugin_list* plugins,
    struct ospath_view  sdk_root,
    const struct ospath_list* extra_plugins)
{
    struct ospath path = empty_ospath();
    struct on_plugin_entry_ctx ctx = { plugins, empty_ospath_view() };

    if (!fs_dir_exists(sdk_root))
    {
        log_sdk_err(
            "SDK root directory {quote:%s} does not existn",
            ospath_view_cstr(sdk_root));
        return -1;
    }

    if (ospath_set(&path, sdk_root) != 0
        || ospath_join_cstr(&path, "plugins") != 0)
    {
        ospath_deinit(path);
        return -1;
    }
    ctx.dir = ospath_view(path);
    fs_list(ospath_view(path), on_plugin_entry, &ctx);

    ospath_deinit(path);

    return 0;
}
