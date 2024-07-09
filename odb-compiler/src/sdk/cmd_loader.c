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
#include "odb-compiler/sdk/dlparser.h"
#include "odb-compiler/sdk/plugin_list.h"

// int
// load_dbpro_commands(
//     struct cmd_list*        commands,
//     plugin_id               plugin_id,
//     const LIEF::PE::Binary* pe,
//     struct ospathc          filepath);
//
// int
// load_odb_commands(
//     struct cmd_list*    commands,
//     plugin_id           plugin_id,
//     const LIEF::Binary* binary,
//     struct ospathc      filepath);

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

static int
on_plugin_string(const char* str, void* user)
{
    return 0;
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
        log_sdk_warn(
            "Failed to load command cache. All plugins will be parsed.\n");
    }

    vec_enumerate(plugins, plugin_id, plugin)
    {
        dlparser_strings(ospathc(plugin->filepath), on_plugin_string, NULL);
#if 0
        if (plugin_is_cached(cached_plugins, plugin_id))
            continue;

        std::unique_ptr<LIEF::Binary> binary(load_binary(plugin, platform));
        if (binary.get() == nullptr)
        {
            log_sdk_warn(
                "Failed to load plugin {quote:%s}. Plugin will be ignored...\n",
                ospath_cstr(plugin->filepath));
            continue;
        }

        log_sdk_progress(
            plugin_id,
            plugin_list_count(plugins),
            "Parsing plugin %s\n",
            utf8_cstr(plugin->name));

        switch (sdk_type)
        {
            case SDK_DBPRO:
                if (binary->format() != LIEF::Binary::FORMATS::PE)
                {
                    log_sdk_warn(
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
#endif
    }

    if (cmd_cache_save(plugins, cmds, sdk_type, arch, platform) != 0)
        log_sdk_warn(
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
