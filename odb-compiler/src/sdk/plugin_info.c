#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"
#include "odb-sdk/ospath.h"

VEC_DEFINE_API(plugin_list, struct plugin_info, 16)

struct on_plugin_entry_ctx
{
    struct plugin_list* plugins;
    struct ospathc      dir;
};

static int
on_plugin_entry_odb(const char* cname, void* user)
{
    struct on_plugin_entry_ctx* ctx = user;
    struct ospathc              name = cstr_ospathc(cname);

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

        if (utf8_set(&plugin->name, ospathc_view(name)) != 0)
            return -1;
        utf8_remove_extension(&plugin->name);
    }

    return 0;
}

static int
on_plugin_entry_dbpro(const char* cname, void* user)
{
    struct on_plugin_entry_ctx* ctx = user;
    struct ospathc              name = cstr_ospathc(cname);

    if (ospath_ends_with_i_cstr(name, ".dll"))
    {
        struct plugin_info* plugin = plugin_list_emplace(ctx->plugins);
        if (plugin == NULL)
            return -1;
        plugin_info_init(plugin);

        if (ospath_set(&plugin->filepath, ctx->dir) != 0)
            return -1;
        if (ospath_join(&plugin->filepath, name) != 0)
            return -1;

        if (utf8_set(&plugin->name, ospathc_view(name)) != 0)
            return -1;
        utf8_remove_extension(&plugin->name);
    }

    return 0;
}

static int
populate_odb(
    struct plugin_list*       plugins,
    struct ospathc            sdk_root,
    const struct ospath_list* extra_plugins)
{
    const char**               plugin_subdir;
    const char*                plugin_subdirs[] = {"plugins", NULL};
    struct ospath              path = empty_ospath();
    struct on_plugin_entry_ctx ctx = {plugins, empty_ospathc()};

    if (!fs_dir_exists(sdk_root))
    {
        log_sdk_err(
            "SDK root directory {quote:%s} does not exist\n",
            ospathc_cstr(sdk_root));
        return -1;
    }

    /* Collect DLL files from subdirectories */
    for (plugin_subdir = plugin_subdirs; *plugin_subdir; ++plugin_subdir)
    {
        if (ospath_set(&path, sdk_root) != 0
            || ospath_join_cstr(&path, *plugin_subdir) != 0)
        {
            ospath_deinit(path);
            return -1;
        }
        ctx.dir = ospathc(path);
        fs_list(ospathc(path), on_plugin_entry_odb, &ctx);
    }

    ospath_deinit(path);
    return 0;
}

static int
populate_dbpro(
    struct plugin_list*       plugins,
    struct ospathc            sdk_root,
    const struct ospath_list* extra_plugins)
{
    const char** plugin_subdir;
    const char*  plugin_subdirs[]
        = {"plugins", "plugins-licensed", "plugins-user", NULL};
    struct ospath              path = empty_ospath();
    struct on_plugin_entry_ctx ctx = {plugins, empty_ospathc()};

    if (!fs_dir_exists(sdk_root))
    {
        log_sdk_err(
            "SDK root directory {quote:%s} does not exist\n",
            ospathc_cstr(sdk_root));
        return -1;
    }

    /* Collect DLL files from subdirectories */
    for (plugin_subdir = plugin_subdirs; *plugin_subdir; ++plugin_subdir)
    {
        if (ospath_set(&path, sdk_root) != 0
            || ospath_join_cstr(&path, *plugin_subdir) != 0)
        {
            ospath_deinit(path);
            return -1;
        }
        ctx.dir = ospathc(path);
        fs_list(ospathc(path), on_plugin_entry_dbpro, &ctx);
    }

    /* Maybe the SDK root directory is pointing to the DBP installation
     * directory instead of the "Compiler" subdirectory. Try to be nice and scan
     * that too. */
    if (vec_count(*plugins) == 0)
    {
        for (plugin_subdir = plugin_subdirs; *plugin_subdir; ++plugin_subdir)
        {
            if (ospath_set(&path, sdk_root) != 0
                || ospath_join_cstr(&path, "Compiler") != 0
                || ospath_join_cstr(&path, *plugin_subdir) != 0)
            {
                ospath_deinit(path);
                return -1;
            }
            ctx.dir = ospathc(path);
            fs_list(ospathc(path), on_plugin_entry_dbpro, &ctx);
        }
    }

    ospath_deinit(path);
    return 0;
}

int
plugin_list_populate(
    struct plugin_list*       plugins,
    enum sdk_type             sdk_type,
    struct ospathc            sdk_root,
    const struct ospath_list* extra_plugins)
{
    switch (sdk_type)
    {
        case SDK_ODB: return populate_odb(plugins, sdk_root, extra_plugins);
        case SDK_DBPRO: return populate_dbpro(plugins, sdk_root, extra_plugins);
    }
    return -1;
}
