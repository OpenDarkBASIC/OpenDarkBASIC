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

    struct utf8 entry_name = empty_utf8();
    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            const std::u16string& u16 = entry.name();
            struct utf16_view     u16v
                = {(const uint16_t*)u16.data(), (utf16_idx)u16.length()};
            if (utf16_to_utf8(&entry_name, u16v) != 0)
            {
                utf8_deinit(entry_name);
                return 0;
            }

            /* String has format: <command>%<type>%<c symbol>%<help>
             * Additionally, if <command> ends with a "[", then the first entry
             * in the type information is the type of the return value instead
             * of the first parameter */
            struct utf8_ref cmd_name, type_str, c_symbol, help;
            utf8_split_ref(
                entry_name.data,
                utf8_ref(entry_name),
                '%',
                &cmd_name,
                &type_str);
            utf8_split_ref(
                entry_name.data, type_str, '%', &type_str, &c_symbol);
            utf8_split_ref(entry_name.data, c_symbol, '%', &c_symbol, &help);

            if (cmd_name.len == 0 || type_str.len == 0 || c_symbol.len == 0)
            {
                log_sdk_warn(
                    "Invalid string table entry in plugin {quote:%s}: %s\n",
                    ospath_view_cstr(filepath),
                    utf8_cstr(entry_name));
                utf8_deinit(entry_name);
                return 0;
            }

            if (cmd_list_add_ref(
                    commands,
                    0,
                    CMD_ARG_VOID,
                    entry_name.data,
                    cmd_name,
                    entry_name.data,
                    c_symbol,
                    entry_name.data,
                    help)
                < 0)
            {
                utf8_deinit(entry_name);
                return -1;
            }
        }
    utf8_deinit(entry_name);

    return 1;
}
