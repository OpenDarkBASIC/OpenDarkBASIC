#include "LIEF/ELF.hpp"
#include "LIEF/MachO.hpp"
#include "LIEF/PE.hpp"
#include "LIEF/PE/ParserConfig.hpp"

extern "C" {
#include "odb-compiler/sdk/cmd_cache.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
}

int
load_dbpro_commands(
    struct cmd_list*        commands,
    plugin_id               plugin_id,
    const LIEF::PE::Binary* pe,
    struct ospathc          filepath);

int
load_odb_commands(
    struct cmd_list*    commands,
    plugin_id           plugin_id,
    const LIEF::Binary* binary,
    struct ospathc      filepath);

struct on_plugin_ctx
{
    plugin_id            current, total;
    struct plugin_ids*   cached_plugins;
    struct cmd_list*     cmds;
    enum sdk_type        sdk_type;
    enum target_platform target_platform;
};

static int
on_plugin(struct plugin_info* plugin, void* user)
{
    std::unique_ptr<LIEF::Binary> binary;
    auto                          ctx = static_cast<on_plugin_ctx*>(user);

    /* Check if current plugin_id has been cached. If so we can skip */
    plugin_id* cached_plugin_id;
    vec_for_each(ctx->cached_plugins, cached_plugin_id)
    {
        if (ctx->current == *cached_plugin_id)
        {
            ctx->current++;
            return 1;
        }
    }

    switch (ctx->target_platform)
    {
        case TARGET_WINDOWS: {
            binary.reset(static_cast<LIEF::Binary*>(
                LIEF::PE::Parser::parse(
                    ospath_cstr(plugin->filepath),
                    LIEF::PE::ParserConfig{
                        false, ///< Parse PE Authenticode signature
                        true,  ///< Parse PE Exports Directory
                        false, ///< Parse PE Import Directory
                        true,  ///< Parse PE resources tree
                        false, ///< Parse PE relocations
                    })
                    .release()));
            break;
        }

        case TARGET_LINUX:
            binary.reset(static_cast<LIEF::Binary*>(
                LIEF::ELF::Parser::parse(ospath_cstr(plugin->filepath))
                    .release()));
            break;

        case TARGET_MACOS:
            log_sdk_err("Loading MachO not implemented\n");
            return -1;
            /*
                binary.reset(static_cast<LIEF::Binary*>(
                    LIEF::MachO::Parser::parse(ospath_cstr(plugin->filepath))
                        .release()));*/
            break;
    }

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
                ctx->cmds,
                ctx->current,
                static_cast<const LIEF::PE::Binary*>(binary.get()),
                ospathc(plugin->filepath)))
            {
                case 1: break;
                case 0:
                    plugin_info_deinit(plugin);
                    ctx->total--;
                    return 0;
                default: return -1;
            }
            break;

        case SDK_ODB:
            switch (load_odb_commands(
                ctx->cmds,
                ctx->current,
                binary.get(),
                ospathc(plugin->filepath)))
            {
                case 1: break;
                case 0:
                    plugin_info_deinit(plugin);
                    ctx->total--;
                    return 0;
                default: return -1;
            }
            break;
    }

    ctx->current++;
    return 1;
}

int
cmd_list_load_from_plugins(
    struct cmd_list*     cmds,
    struct plugin_list*  plugins,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform)
{
    struct on_plugin_ctx ctx
        = {0, (plugin_id)plugins->count, NULL, cmds, sdk_type, platform};
    plugin_ids_init(&ctx.cached_plugins);

    log_sdk_progress(0, 1, "Loading command cache");
    if (cmd_cache_load(
            &ctx.cached_plugins, plugins, cmds, sdk_type, arch, platform)
        != 0)
        log_sdk_note("All plugins will be parsed.\n");

    log_sdk_progress(0, 1, "Parsing plugins");
    if (plugin_list_retain(plugins, on_plugin, &ctx) != 0)
        goto parse_plugins_failed;

    if (cmd_cache_save(plugins, cmds, sdk_type, arch, platform) != 0)
        log_sdk_note("All plugins will be parsed next time.\n");

    plugin_ids_deinit(ctx.cached_plugins);
    return 0;

parse_plugins_failed:
    plugin_ids_deinit(ctx.cached_plugins);
    return -1;
}
