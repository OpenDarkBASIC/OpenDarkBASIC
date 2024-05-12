extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/utf8.h"
}

#include "LIEF/PE.hpp"
#include "LIEF/PE/Binary.hpp"
#include "LIEF/PE/ResourceNode.hpp"

int
load_dbpro_commands(
    struct cmd_list*        commands,
    const LIEF::PE::Binary* pe,
    struct ospath_view      filepath)
{
    if (pe->resources() == nullptr)
    {
        log_sdk_warn(
            "No resources found in plugin {quote:%s}.\n",
            ospath_view_cstr(filepath));
        return 0;
    }

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
    {
        log_sdk_warn(
            "Missing string table in plugin {quote:%s}.\n",
            ospath_view_cstr(filepath));
        return 0;
    }
    if (stringTableNodeIt->childs().size() == 0
        || stringTableNodeIt->childs()[0].childs().size() == 0)
    {
        log_sdk_warn(
            "Malformed string table structure in plugin {quote:%s}.\n",
            ospath_view_cstr(filepath));
        return 0;
    }

    struct utf8 entry = empty_utf8();
    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            const std::u16string& u16 = entry.name();
            struct utf16_view     u16v
                = {(const uint16_t*)u16.data(), (utf16_idx)u16.length()};
            if (utf16_to_utf8(&entry, u16v) != 0)
            {
                utf8_deinit(entry);
                return 0;
            }

            struct utf8_view cmd_name, type_str;
            utf8_split(utf8_view(entry), '%' &cmd_name, &type_str);
            if (cmd_name.len == 0)
            {
                log_sdk_warn(
                    "Invalid string table entry in plugin {quote:%s}: {emph:%s}\n",
                    ospath_view_cstr(filepath),
                    utf8_cstr(entry));
                utf8_deinit(entry);
                return 0;
            }

            if (cmd_list_add(
                    commands,
                    0,
                    CMD_ARG_VOID,
                    utf8_view(entry),
                    empty_utf8_view(),
                    empty_utf8_view())
                < 0)
            {
                utf8_deinit(entry);
                return -1;
            }
        }
    utf8_deinit(entry);

    return 1;
}
