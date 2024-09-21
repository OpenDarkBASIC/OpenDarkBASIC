#include "odb-compiler/sdk/plugin_list.h"
#include "odb-util/fs.h"
#include "odb-util/log.h"
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
            break;
        }

    return 0;
}

int
plugin_list_populate(
    struct plugin_list**      plugins,
    enum sdk_type             sdk_type,
    enum target_platform      target_platform,
    struct ospathc            sdk_root,
    struct ospath_list*       extra_plugins)
{
    const char*                subdir;
    const char**               psubdir;
    const char**               plugin_subdirs;
    struct ospathc             pathc;
    struct ospath              path = empty_ospath();
    struct on_plugin_entry_ctx ctx = {plugins, empty_ospathc(), NULL};

    if (!fs_dir_exists(sdk_root))
    {
        log_cmd_err(
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
        log_dbg("[sdk] ", "Extra plugin: %s\n", ospathc_cstr(pathc));

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
