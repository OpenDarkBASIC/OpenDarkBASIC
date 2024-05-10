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

    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            //for (uint16_t c : entry.name())
            //{
            //    char l = (c & 0xFF);
            //    char h = ((c >> 8) & 0xFF);
            //    putc(std::isalnum(l) ? l : '.', stdout);
            //    putc(std::isalnum(h) ? h : '.', stdout);
            //}
            //puts("");

            struct utf8 name = empty_utf8();
            if (utf16_to_utf8(
                    &name, cstr_utf16_view((uint16_t*)entry.name().c_str()))
                == 0)
            {
                //printf("%s\n", utf8_cstr(name));
            }
            utf8_deinit(name);
        }

    return 0;
}
