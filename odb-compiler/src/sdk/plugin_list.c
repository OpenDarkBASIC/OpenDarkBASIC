#include "odb-compiler/sdk/plugin_list.h"
#include "odb-util/fs.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/ospath.h"
#include "odb-util/ospath_list.h"
#include "odb-util/utf8_list.h"

VEC_DEFINE_API(plugin_list, struct plugin_info, 16)

const char* dbpro_plugin_subdirs[]
    = {"plugins",
       "plugins-licensed",
       "plugins-user",
       /* Maybe the SDK root directory is pointing to the DBP
        * installation directory instead of the "Compiler"
        * subdirectory. Try to be nice and scan that too. */
       "Compiler/plugins",
       "Compiler/plugins-licensed",
       "Compiler/plugins-user",
       NULL};
static const char* odb_plugin_subdirs[] = {"plugins", NULL};

static const char* windows_extensions[] = {".dll", NULL};
static const char* linux_extensions[] = {".so", NULL};
static const char* macos_extensions[]
    = {".dylib", ".dynlib", ".framework", NULL};

struct on_plugin_entry_ctx
{
    struct plugin_list** plugins;
    struct ospathc       dir;
    const char**         extensions;
};

static int
on_plugin_entry(const char* cname, void* user)
{
    const char**                ext;
    struct on_plugin_entry_ctx* ctx = user;
    struct ospathc              name = cstr_ospathc(cname);

    for (ext = ctx->extensions; *ext; ++ext)
        if (ospath_ends_with_i_cstr(name, *ext))
        {
            struct plugin_info* plugin2;
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
            utf8_remove_ext(&plugin->name);

            vec_for_each(*ctx->plugins, plugin2)
            {
                if (utf8_equal(
                        utf8_view(plugin2->name), utf8_view(plugin->name))
                    && !utf8_equal(
                        ospath_view(plugin2->filepath),
                        ospath_view(plugin->filepath)))
                {
                    return log_err(
                        "Same plugin added with different path.\n"
                        "  Existing: {quote:%s}\n"
                        "  New     : {quote:%s}\n",
                        ospath_cstr(plugin2->filepath),
                        ospath_cstr(plugin->filepath));
                }
            }
            break;
        }

    return 0;
}

#if defined(ODBUTIL_MEM_DEBUGGING)
void
mem_acquire_plugin_list(struct plugin_list* plugins)
{
    struct plugin_info* plugin;
    if (plugins == NULL)
        return;

    mem_acquire(
        plugins,
        offsetof(struct plugin_list, data)
            + sizeof(struct plugin_info) * plugins->capacity);
    vec_for_each(plugins, plugin)
    {
        mem_acquire(plugin->name.data, sizeof(plugin->name.len));
        mem_acquire(
            plugin->filepath.str.data, sizeof(plugin->filepath.str.len));
    }
}
void
mem_release_plugin_list(struct plugin_list* plugins)
{
    struct plugin_info* plugin;
    if (plugins == NULL)
        return;

    vec_for_each(plugins, plugin)
    {
        mem_release(plugin->name.data);
        mem_release(plugin->filepath.str.data);
    }
    mem_release(plugins);
}
#endif

plugin_id
plugin_list_add_or_get(struct plugin_list** plugins, struct ospath* filepath)
{
    plugin_id           plugin_id;
    struct plugin_info* plugin;
    struct ospath       name = empty_ospath();

    if (utf8_set(&name.str, utf8_view(filepath->str)) != 0)
        return -1;
    ospath_filename(&name);
    ospath_remove_ext(&name);
    vec_enumerate(*plugins, plugin_id, plugin)
    {
        if (utf8_equal(utf8_view(plugin->name), ospath_view(name)))
        {
            if (!utf8_equal(
                    ospath_view(plugin->filepath), utf8_view(filepath->str)))
            {
                return log_err(
                    "Same plugin added with different path.\n"
                    "  Existing: {quote:%s}\n"
                    "  New     : {quote:%s}\n",
                    ospath_cstr(plugin->filepath),
                    ospath_cstr(*filepath));
            }

            ospath_deinit(name);
            return plugin_id;
        }
    }

    plugin = plugin_list_emplace(plugins);
    if (plugin == NULL)
        return -1;
    plugin_info_init(plugin);

    plugin->name = name.str;
    plugin->filepath = *filepath;
    filepath->str = empty_utf8();

    log_dbg("Added plugin: %s\n", ospath_cstr(plugin->filepath));

    return plugin_list_count(*plugins) - 1;
}

int
plugin_list_populate(
    struct plugin_list** plugins,
    enum sdk_type        sdk_type,
    enum target_platform target_platform,
    struct ospathc       sdk_root,
    struct ospath_list*  extra_plugins)
{
    const char**               psubdir;
    const char**               plugin_subdirs;
    struct ospathc             pathc;
    struct ospath              path = empty_ospath();
    struct on_plugin_entry_ctx ctx = {plugins, empty_ospathc(), NULL};

    if (!fs_dir_exists(sdk_root))
    {
        log_err(
            "SDK root directory {quote:%s} does not exist\n",
            ospathc_cstr(sdk_root));
        return -1;
    }

    switch (sdk_type)
    {
        case SDK_ODB: plugin_subdirs = odb_plugin_subdirs; break;
        case SDK_DBPRO: plugin_subdirs = dbpro_plugin_subdirs; break;
    }
    switch (target_platform)
    {
        case TARGET_WINDOWS: ctx.extensions = windows_extensions; break;
        case TARGET_MACOS: ctx.extensions = macos_extensions; break;
        case TARGET_LINUX: ctx.extensions = linux_extensions; break;
    }

    for (psubdir = plugin_subdirs; *psubdir; ++psubdir)
    {
        if (ospath_set(&path, sdk_root) != 0
            || ospath_join_cstr(&path, *psubdir) != 0)
        {
            goto fail;
        }

        ctx.dir = ospathc(path);
        if (fs_list(ospathc(path), on_plugin_entry, &ctx) < 0)
            goto fail;
    }

    ospath_for_each(extra_plugins, pathc)
    {
        log_dbg("Extra plugin: %s\n", ospathc_cstr(pathc));

        ctx.dir = pathc;
        if (fs_list(pathc, on_plugin_entry, &ctx) < 0)
            goto fail;
    }

    ospath_deinit(path);
    return 0;

fail:
    ospath_deinit(path);
    return -1;
}
