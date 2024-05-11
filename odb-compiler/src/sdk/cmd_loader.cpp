#include "LIEF/PE.hpp"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
}

int
load_dbpro_commands(
    struct cmd_list*        commands,
    const LIEF::PE::Binary* pe,
    struct ospath_view      filepath);

struct on_plugin_ctx
{
    int              current, total;
    struct cmd_list* commands;
    enum sdk_type    sdk_type;
};

static int
on_plugin(struct plugin_info* plugin, void* user)
{
    auto                          ctx = static_cast<on_plugin_ctx*>(user);
    std::unique_ptr<LIEF::Binary> binary
        = LIEF::Parser::parse(ospath_cstr(plugin->filepath));
    if (binary.get() == nullptr)
    {
        log_sdk_warn(
            "Failed to load plugin {quote:%s}. Plugin will be ignored...\n",
            ospath_cstr(plugin->filepath));
        plugin_info_deinit(plugin);
        ctx->total--;
        return 0;
    }

    log_sdk_progress(
        ctx->current,
        ctx->total,
        "Parsing plugin %s\n",
        utf8_cstr(plugin->name));

    switch (ctx->sdk_type)
    {
        case SDK_DBPRO:
            if (binary->format() != LIEF::Binary::FORMATS::PE)
            {
                log_sdk_warn(
                    "{quote:%s} is not a valid PE file. Plugin will be "
                    "ignored...\n",
                    ospath_cstr(plugin->filepath));
                plugin_info_deinit(plugin);
                ctx->total--;
                return 0;
            }
            switch (load_dbpro_commands(
                ctx->commands,
                static_cast<const LIEF::PE::Binary*>(binary.get()),
                ospath_view(plugin->filepath)))
            {
                case 1: break;
                case 0:
                    plugin_info_deinit(plugin);
                    ctx->total--;
                    return 0;
                default: return -1;
            }
            break;

        case SDK_ODB: break;
    }

    ctx->current++;
    return 1;
}

int
cmd_list_load_from_plugins(
    struct cmd_list*   commands,
    enum sdk_type      sdk_type,
    struct plugin_list plugins)
{
    struct on_plugin_ctx ctx = {0, vec_count(plugins), commands, sdk_type};
    if (plugin_list_retain(plugins, on_plugin, &ctx) != 0)
        return -1;

    return 0;
}
