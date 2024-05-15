extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/utf8.h"
}

#include "LIEF/PE.hpp"
#include "LIEF/PE/Binary.hpp"
#include "LIEF/PE/ResourceNode.hpp"

static enum cmd_param_type
convert_char_to_return_type(char c)
{
    switch ((enum cmd_param_type)c)
    {
        case CMD_PARAM_VOID:
        case CMD_PARAM_LONG:
        case CMD_PARAM_DWORD:
        case CMD_PARAM_INTEGER:
        case CMD_PARAM_WORD:
        case CMD_PARAM_BYTE:
        case CMD_PARAM_BOOLEAN:
        case CMD_PARAM_FLOAT:
        case CMD_PARAM_DOUBLE:
        case CMD_PARAM_STRING:
        case CMD_PARAM_ARRAY:
        case CMD_PARAM_LABEL:
        case CMD_PARAM_DABEL:
        case CMD_PARAM_ANY: return (enum cmd_param_type)c;

        case CMD_PARAM_USER_DEFINED_VAR_PTR: break;
    }
    return (enum cmd_param_type)0;
}

static enum cmd_param_type
convert_char_to_param_type(char c)
{
    switch ((enum cmd_param_type)c)
    {
        case CMD_PARAM_VOID:
        case CMD_PARAM_LONG:
        case CMD_PARAM_DWORD:
        case CMD_PARAM_INTEGER:
        case CMD_PARAM_WORD:
        case CMD_PARAM_BYTE:
        case CMD_PARAM_BOOLEAN:
        case CMD_PARAM_FLOAT:
        case CMD_PARAM_DOUBLE:
        case CMD_PARAM_STRING:
        case CMD_PARAM_ARRAY:
        case CMD_PARAM_LABEL:
        case CMD_PARAM_DABEL:
        case CMD_PARAM_ANY: return (enum cmd_param_type)c;

        case CMD_PARAM_USER_DEFINED_VAR_PTR: break;
    }
    return (enum cmd_param_type)0;
}

int
load_dbpro_commands(
    struct cmd_list*        commands,
    plugin_ref              plugin_id,
    const LIEF::PE::Binary* pe,
    struct ospathc          filepath)
{
    if (pe->resources() == nullptr)
    {
        log_sdk_warn(
            "No resources found in plugin {emph:%s}.\n",
            ospathc_cstr(filepath));
        return 0;
    }

    /*
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
            "Missing string table in plugin {emph:%s}.\n",
            ospathc_cstr(filepath));
        return 0;
    }
    if (stringTableNodeIt->childs().size() == 0
        || stringTableNodeIt->childs()[0].childs().size() == 0)
    {
        log_sdk_warn(
            "Malformed string table structure in plugin {emph:%s}.\n",
            ospathc_cstr(filepath));
        return 0;
    }*/

    struct utf8 entry_str = empty_utf8();
    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            const std::u16string& u16 = entry.name();
            struct utf16_view     u16v
                = {(const uint16_t*)u16.data(), (utf16_idx)u16.length()};
            if (utf16_to_utf8(&entry_str, u16v) != 0)
                goto bad_plugin;

            /* String has format: <command>%<type>%<c symbol>%<help>
             * Split on '%' into the relevant parts. */
            struct utf8_span cmd_name, type_str, c_symbol, doc;
            utf8_split(
                entry_str.data,
                utf8_span(entry_str),
                '%',
                &cmd_name,
                &type_str);
            utf8_split(entry_str.data, type_str, '%', &type_str, &c_symbol);
            utf8_split(entry_str.data, c_symbol, '%', &c_symbol, &doc);
            if (cmd_name.len == 0 || type_str.len == 0 || c_symbol.len == 0)
            {
                log_sdk_warn(
                    "Invalid string table entry {quote:%s} in plugin "
                    "{emph:%s}\n",
                    utf8_cstr(entry_str),
                    ospathc_cstr(filepath));
                goto bad_plugin;
            }

            /* If <command> ends with a "[", then the first entry in the type
             * information string is the type of the return value instead of
             * the first parameter. Otherwise the return value is void. */
            enum cmd_param_type return_type = CMD_PARAM_VOID;
            if (entry_str.data[cmd_name.off + cmd_name.len - 1] == '[')
            {
                char type_char = entry_str.data[type_str.off];
                return_type = convert_char_to_return_type(type_char);
                if (return_type == 0)
                {
                    log_sdk_warn(
                        "Invalid command return type {quote:%c} in string "
                        "{quote:%s} in plugin {emph:%s}\n",
                        type_char,
                        utf8_cstr(entry_str),
                        ospathc_cstr(filepath));
                    goto bad_plugin;
                }

                type_str.off++;
                type_str.len--;
                cmd_name.len--;
            }

            /* Command names are case-insensitive. By convention we store them
             * in lower case in the command list for this reason */
            utf8_tolower_span(entry_str.data, cmd_name);

            cmd_idx cmd = cmd_list_add(
                commands,
                plugin_id,
                return_type,
                utf8_span_view(entry_str.data, cmd_name),
                utf8_span_view(entry_str.data, c_symbol),
                utf8_span_view(entry_str.data, doc));
            if (cmd < 0)
                goto critical_error;

            /* Parse and add each parameter type to the command. If a character
             * is proceeded by an asterisk "*", then it is an out parameter */
            struct utf8_span param_doc;
            for (utf8_idx i = 0; i != type_str.len; ++i)
            {
                char type_char = entry_str.data[type_str.off + i];
                enum cmd_param_direction direction = CMD_PARAM_IN;
                enum cmd_param_type      type
                    = convert_char_to_param_type(type_char);

                utf8_split(entry_str.data, doc, ',', &param_doc, &doc);

                if (type == 0)
                {
                    log_sdk_warn(
                        "Invalid command argument type {quote:%c} in string "
                        "{quote:%s} in plugin {emph:%s}\n",
                        type_char,
                        utf8_cstr(entry_str),
                        ospathc_cstr(filepath));
                    /* Skip command, but plugin is still usable hopefully */
                    cmd_list_erase(commands, cmd);
                    continue;
                }

                if (i + 1 < type_str.len
                    && entry_str.data[type_str.off + i + 1] == '*')
                {
                    i++;
                    direction = CMD_PARAM_OUT;
                }

                if (cmd_add_param(
                        commands,
                        cmd,
                        type,
                        direction,
                        utf8_span_view(entry_str.data, param_doc))
                    != 0)
                {
                    goto critical_error;
                }
            }
        }

    utf8_deinit(entry_str);
    return 1;

bad_plugin:
    utf8_deinit(entry_str);
    return 0;
critical_error:
    utf8_deinit(entry_str);
    return -1;
}
