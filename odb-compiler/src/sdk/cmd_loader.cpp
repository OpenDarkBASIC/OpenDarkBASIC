#include "LIEF/ELF.hpp"
#include "LIEF/MachO.hpp"
#include "LIEF/PE.hpp"
#include "LIEF/PE/ParserConfig.hpp"
#include "odb-compiler/sdk/sdk.h"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
}

int
load_dbpro_commands(
    struct cmd_list*        commands,
    plugin_ref              plugin_id,
    const LIEF::PE::Binary* pe,
    struct ospathc          filepath);

int
load_odb_commands(
    struct cmd_list*    commands,
    plugin_ref          plugin_id,
    const LIEF::Binary* binary,
    struct ospathc      filepath);

struct on_plugin_ctx
{
    plugin_ref                current, total;
    struct cmd_list*          commands;
    enum sdk_type             sdk_type;
    enum odb_codegen_platform target_platform;
};

static int
on_plugin(struct plugin_info* plugin, void* user)
{
    std::unique_ptr<LIEF::Binary> binary;
    auto                          ctx = static_cast<on_plugin_ctx*>(user);
    switch (ctx->target_platform)
    {
        case ODB_CODEGEN_WINDOWS: {
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

        case ODB_CODEGEN_LINUX:
            binary.reset(static_cast<LIEF::Binary*>(
                LIEF::ELF::Parser::parse(ospath_cstr(plugin->filepath))
                    .release()));
            break;

        case ODB_CODEGEN_MACOS:
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
                ctx->commands,
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
                ctx->commands,
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
    struct cmd_list*          commands,
    enum sdk_type             sdk_type,
    enum odb_codegen_platform target_platform,
    struct plugin_list        plugins)
{
    struct on_plugin_ctx ctx = {
        0, (plugin_ref)vec_count(plugins), commands, sdk_type, target_platform};
    if (plugin_list_retain(plugins, on_plugin, &ctx) != 0)
        return -1;

    return 0;
}
