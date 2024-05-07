#include "LIEF/PE.hpp"

extern "C" {
#include "odb-compiler/sdk/command_list.h"
#include "odb-compiler/sdk/plugin_list.h"
}

int
load_dbpro_commands(struct cmd_list* commands, const LIEF::PE::Binary* pe);

int
cmd_list_load_from_plugins(
    struct cmd_list*          commands,
    enum sdk_type             sdk_type,
    const struct plugin_list* plugins)
{
    struct plugin_info* plugin;
    vec_for_each(plugins, plugin)
    {
        std::unique_ptr<LIEF::Binary> binary
            = LIEF::Parser::parse(ospath_cstr(plugin->filepath));
        if (binary.get() == nullptr)
        {
            log_sdk_warn(
                "Failed to load plugin {quote:%s}. Plugin will be ignored...\n",
                ospath_cstr(plugin->filepath));
            // TODO: remove plugin from vector here
            continue;
        }

        switch (sdk_type)
        {
            case SDK_DBPRO:
                if (binary->format() != LIEF::Binary::FORMATS::PE)
                {
                    log_sdk_warn(
                        "{quote:%s} is not a valid PE file. Plugin will be "
                        "ignored...\n",
                        ospath_cstr(plugin->filepath));
                    // TODO: remove plugin from vector here
                    continue;
                }
                if (load_dbpro_commands(
                        commands,
                        static_cast<const LIEF::PE::Binary*>(binary.get()))
                    != 0)
                {
                    // TODO: remove plugin from vector here
                }
                break;

            case SDK_ODB: break;
        }
    }

    return 0;
}
