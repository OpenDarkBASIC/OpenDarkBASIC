extern "C" {
#include "odb-compiler/sdk/command_loader.h"
#include "odb-compiler/sdk/plugin_info.h"
#include "odb-sdk/utf8.h"
}

#include "LIEF/PE.hpp"
#include "LIEF/PE/Binary.hpp"
#include "LIEF/PE/ResourceNode.hpp"

static int
load_dbpro_commands(struct command_list* commands, const LIEF::PE::Binary* pe)
{
    if (pe->resources() == nullptr)
        return -1;

    const auto& nodes = pe->resources()->childs();
    auto        stringTableNodeIt = std::find_if(
        std::begin(nodes),
        std::end(nodes),
        [](const LIEF::PE::ResourceNode& node)
        {
            return static_cast<LIEF::PE::ResourcesManager::TYPE>(node.id())
                   == LIEF::PE::ResourcesManager::TYPE::STRING;
        });
    if (stringTableNodeIt == std::end(nodes))
        return -1;
    if (stringTableNodeIt->childs().size() == 0
        || stringTableNodeIt->childs()[0].childs().size() == 0)
        return -1;

    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            struct utf8 name = empty_utf8();
            if (utf16_to_utf8(
                    &name, cstr_utf16_view((uint16_t*)entry.name().c_str()))
                == 0)
                printf("%s\n", utf8_cstr(name));
            utf8_deinit(name);
        }

    return 0;
}

int
commands_load_all(
    struct command_list*      commands,
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
