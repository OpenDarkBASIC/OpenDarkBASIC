#include "odb-compiler/commands/plugin_info.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/log.h"

VEC_DEFINE_API(plugin_list, struct plugin_info, 16)

static int
on_plugin_entry(const char* name, void* user)
{
    log_dbg("dbg: ", "%s\n", name);
    return 0;
}

int
plugin_list_populate(
    struct plugin_list* plugins,
    struct ospath_view  sdk_root,
    const struct ospath_list*  extra_plugin_dirs)
{
    struct ospath path = ospath();

    if (!fs_dir_exists(sdk_root))
    {
        log_sdk_err(
            "SDK root directory {quote:%s} does not exist\n",
            ospath_view_cstr(sdk_root));
        return -1;
    }

    if (ospath_set(&path, sdk_root) != 0
        || ospath_join_cstr(&path, "plugins") != 0)
        return -1;
    fs_list(ospath_view(path), on_plugin_entry, NULL);

    return 0;
}
