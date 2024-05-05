extern "C" {
#include "odb-compiler/sdk/command_loader.h"
#include "odb-compiler/sdk/plugin_info.h"
}

#include "LIEF/ELF.hpp"

int
commands_load_all(
    struct command_list* commands, const struct plugin_list* plugins)
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
/*
        for (const LIEF::Symbol& symbol : binary->getStringTable())
            log_dbg("", "%s\n", symbol.name().c_str());*/
    }

    return 0;
}
