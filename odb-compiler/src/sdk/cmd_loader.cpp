/* TODO: Remove this once LIEF Is gone */
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

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

static int
plugin_is_cached(const struct plugin_ids* cached_plugins, plugin_id id)
{
    const plugin_id* cached_id;
    vec_for_each(cached_plugins, cached_id)
    {
        if (id == *cached_id)
            return 1;
    }
    return 0;
}

static LIEF::Binary*
load_binary(const struct plugin_info* plugin, enum target_platform platform)
{
    switch (platform)
    {
        case TARGET_WINDOWS: {
            return static_cast<LIEF::Binary*>(
                LIEF::PE::Parser::parse(
                    ospath_cstr(plugin->filepath),
                    LIEF::PE::ParserConfig{
                        false, ///< Parse PE Authenticode signature
                        true,  ///< Parse PE Exports Directory
                        false, ///< Parse PE Import Directory
                        true,  ///< Parse PE resources tree
                        false, ///< Parse PE relocations
                    })
                    .release());
        }

        case TARGET_LINUX:
            return static_cast<LIEF::Binary*>(
                LIEF::ELF::Parser::parse(ospath_cstr(plugin->filepath))
                    .release());

        case TARGET_MACOS:
            log_cmd_err("Loading MachO not implemented\n");
            return nullptr;
            /*
                binary.reset(static_cast<LIEF::Binary*>(
                    LIEF::MachO::Parser::parse(ospath_cstr(plugin->filepath))
                        .release()));*/
    }

    return nullptr;
}

int
cmd_list_load_from_plugins(
    struct cmd_list*          cmds,
    const struct plugin_list* plugins,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform)
{
    struct plugin_ids*        cached_plugins;
    const struct plugin_info* plugin;
    plugin_id                 plugin_id;

    plugin_ids_init(&cached_plugins);

    log_cmd_progress(0, plugin_list_count(plugins), "Loading command cache");
    if (cmd_cache_load(&cached_plugins, plugins, cmds, sdk_type, arch, platform)
        != 0)
    {
        log_cmd_warn(
            "Failed to load command cache. All plugins will be parsed.\n");
    }

    vec_enumerate(plugins, plugin_id, plugin)
    {
        if (plugin_is_cached(cached_plugins, plugin_id))
            continue;

        std::unique_ptr<LIEF::Binary> binary(load_binary(plugin, platform));
        if (binary.get() == nullptr)
        {
            log_cmd_warn(
                "Failed to load plugin {quote:%s}. Plugin will be ignored...\n",
                ospath_cstr(plugin->filepath));
            continue;
        }

        log_cmd_progress(
            plugin_id,
            plugin_list_count(plugins),
            "Parsing plugin %s\n",
            utf8_cstr(plugin->name));

        switch (sdk_type)
        {
            case SDK_DBPRO:
                if (binary->format() != LIEF::Binary::FORMATS::PE)
                {
                    log_cmd_warn(
                        "{quote:%s} is not a valid PE file. Plugin will be "
                        "ignored...\n",
                        ospath_cstr(plugin->filepath));
                    continue;
                }
                if (load_dbpro_commands(
                        cmds,
                        plugin_id,
                        static_cast<const LIEF::PE::Binary*>(binary.get()),
                        ospathc(plugin->filepath))
                    != 0)
                {
                    goto fatal_error;
                }
                break;

            case SDK_ODB:
                if (load_odb_commands(
                        cmds,
                        plugin_id,
                        binary.get(),
                        ospathc(plugin->filepath))
                    != 0)
                {
                    goto fatal_error;
                }
                break;
        }
    }

    if (cmd_cache_save(plugins, cmds, sdk_type, arch, platform) != 0)
        log_cmd_warn(
            "Failed to save command cache. All plugins will be parsed next "
            "time.\n");

    plugin_ids_deinit(cached_plugins);
    return 0;

fatal_error:
    plugin_ids_deinit(cached_plugins);
    return -1;
}

#if __GNUC__
#pragma GCC diagnostic pop
#endif
