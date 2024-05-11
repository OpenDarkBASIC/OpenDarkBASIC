extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
}

#include "LIEF/PE.hpp"
#include "LIEF/PE/Binary.hpp"
#include "LIEF/PE/ResourceNode.hpp"

int
load_dbpro_commands(struct cmd_list* commands, const LIEF::PE::Binary* pe)
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
    
    struct utf8 name = empty_utf8();
    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            if (utf16_to_utf8(
                    &name, cstr_utf16_view((uint16_t*)entry.name().c_str()))
                == 0)
            {
                cmd_list_add(
                    commands,
                    0,
                    CMD_ARG_VOID,
                    utf8_view(name),
                    empty_utf8_view(),
                    empty_utf8_view());
            }
        }
    utf8_deinit(name);

    return 0;
}
