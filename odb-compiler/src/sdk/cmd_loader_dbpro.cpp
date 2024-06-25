extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/type.h"
#include "odb-sdk/utf8.h"
}

#include "LIEF/PE.hpp"
#include "LIEF/PE/Binary.hpp"
#include "LIEF/PE/ResourceNode.hpp"

static enum type
convert_char_to_return_type(char c)
{
    switch (c)
    {
        case '0': return TYPE_VOID;
        case 'R': return TYPE_DOUBLE_INTEGER;
        case 'D': return TYPE_DWORD;
        case 'L': return TYPE_INTEGER;
        case 'W': return TYPE_WORD;
        case 'Y': return TYPE_BYTE;
        case 'B': return TYPE_BOOLEAN;
        case 'F': return TYPE_FLOAT;
        case 'O': return TYPE_DOUBLE;
        case 'S': return TYPE_STRING;
        case 'H': return TYPE_ARRAY;
        case 'P': return TYPE_LABEL;
        case 'Q': return TYPE_DABEL;
        case 'X': return TYPE_ANY;

        case 'E': break;
    }

    return TYPE_INVALID;
}

static enum type
convert_char_to_param_type(char c)
{
    switch (c)
    {
        case '0': return TYPE_VOID;
        case 'R': return TYPE_DOUBLE_INTEGER;
        case 'D': return TYPE_DWORD;
        case 'L': return TYPE_INTEGER;
        case 'W': return TYPE_WORD;
        case 'Y': return TYPE_BYTE;
        case 'B': return TYPE_BOOLEAN;
        case 'F': return TYPE_FLOAT;
        case 'O': return TYPE_DOUBLE;
        case 'S': return TYPE_STRING;
        case 'H': return TYPE_ARRAY;
        case 'P': return TYPE_LABEL;
        case 'Q': return TYPE_DABEL;
        case 'X': return TYPE_ANY;

        case 'E': break;
    }

    return TYPE_INVALID;
}

static int
looks_like_command_string(struct utf8 entry_str, struct utf8_span type_str)
{
    /* Only log a warning if the string "looks" like a command
     * string. This is an attempt to filter out resource strings
     * that are used for other purposes. */
    return utf8_count_substrings_cstr(utf8_view(entry_str), "%") >= 2
           && utf8_count_substrings_cstr(
                  utf8_span_view(entry_str.data, type_str), " ")
                  == 0;
}

int
load_dbpro_commands(
    struct cmd_list*        commands,
    plugin_id               plugin_id,
    const LIEF::PE::Binary* pe,
    struct ospathc          filepath)
{
    if (pe->resources() == nullptr)
    {
        log_cmd_warn(
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
        log_cmd_warn(
            "Missing string table in plugin {emph:%s}.\n",
            ospathc_cstr(filepath));
        return 0;
    }
    if (stringTableNodeIt->childs().size() == 0
        || stringTableNodeIt->childs()[0].childs().size() == 0)
    {
        log_cmd_warn(
            "Malformed string table structure in plugin {emph:%s}.\n",
            ospathc_cstr(filepath));
        return 0;
    }*/

    struct utf8 entry_str = empty_utf8();
    if (auto resmgr = pe->resources_manager())
        for (const auto& entry : resmgr.value().string_table())
        {
            enum type             return_type;
            cmd_id                cmd;
            const std::u16string& u16 = entry.name();
            struct utf16_view     u16v
                = {(const uint16_t*)u16.data(), (utf16_idx)u16.length()};
            if (utf16_to_utf8(&entry_str, u16v) != 0)
                goto critical_error;

            /* String has format: <command>%<type>%<c symbol>%<help>
             * Split on '%' into the relevant parts. */
            struct utf8_span cmd_name, type_str, c_symbol, db_params;
            utf8_split(
                entry_str.data,
                utf8_span(entry_str),
                '%',
                &cmd_name,
                &type_str);
            utf8_split(entry_str.data, type_str, '%', &type_str, &c_symbol);
            utf8_split(entry_str.data, c_symbol, '%', &c_symbol, &db_params);
            if (cmd_name.len == 0 || type_str.len == 0 || c_symbol.len == 0)
            {
                if (looks_like_command_string(entry_str, type_str))
                {
                    log_cmd_warn(
                        "Invalid string table entry {quote:%s} in plugin "
                        "{emph:%s}\n",
                        utf8_cstr(entry_str),
                        ospathc_cstr(filepath));
                }
                goto bad_command;
            }

            /* DBPro includes "fake" command keywords so the parser and editor
             * highlighting is able to function. ODB manages the keywords
             * separately, so we need to ignore these. The C symbol in these
             * cases is either empty or contains "??". */
            if (entry_str.data[c_symbol.off] == '?'
                && entry_str.data[c_symbol.off + 1] == '?')
                goto bad_command;

            /* If <command> ends with a "[", then the first entry in the type
             * information string is the type of the return value instead of
             * the first parameter. Otherwise the return value is void. */
            return_type = TYPE_VOID;
            if (entry_str.data[cmd_name.off + cmd_name.len - 1] == '[')
            {
                char type_char = entry_str.data[type_str.off];
                return_type = convert_char_to_return_type(type_char);
                if (return_type == TYPE_INVALID)
                {
                    if (looks_like_command_string(entry_str, type_str))
                    {
                        log_cmd_warn(
                            "Invalid command return type {quote:%c} in string "
                            "{quote:%s} in plugin {emph:%s}\n",
                            type_char,
                            utf8_cstr(entry_str),
                            ospathc_cstr(filepath));
                    }
                    goto bad_command;
                }

                type_str.off++;
                type_str.len--;
                cmd_name.len--;
            }

            /* Command names are case-insensitive. By convention we store them
             * in upper case in the command list for this reason */
            utf8_toupper_span(entry_str.data, cmd_name);

            cmd = cmd_list_add(
                commands,
                plugin_id,
                return_type,
                utf8_span_view(entry_str.data, cmd_name),
                utf8_span_view(entry_str.data, c_symbol));
            if (cmd < 0)
                goto critical_error;

            /* Parse and add each parameter type to the command. If a character
             * is proceeded by an asterisk "*", then it is an out parameter.
             * Some commands have void parameters. These need to be skipped so
             * that the parameter count is correct */
            struct utf8_span db_param_name;
            for (utf8_idx i = 0; i != type_str.len; ++i)
            {
                char type_char = entry_str.data[type_str.off + i];
                enum cmd_param_direction direction = CMD_PARAM_IN;
                enum type type = convert_char_to_param_type(type_char);

                utf8_split(
                    entry_str.data, db_params, ',', &db_param_name, &db_params);
                db_param_name
                    = utf8_strip_span(entry_str.data, db_param_name, " ");

                if (type == TYPE_INVALID)
                {
                    if (looks_like_command_string(entry_str, type_str))
                    {
                        log_cmd_warn(
                            "Invalid command argument type {quote:%c} in "
                            "string "
                            "{quote:%s} in plugin {emph:%s}\n",
                            type_char,
                            utf8_cstr(entry_str),
                            ospathc_cstr(filepath));
                    }

                    /* Skip command, but plugin is still usable hopefully */
                    cmd_list_erase(commands, cmd);
                    goto bad_command;
                }

                /* A void parameter is no parameter */
                if (type == TYPE_VOID)
                    continue;

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
                        utf8_span_view(entry_str.data, db_param_name))
                    != 0)
                {
                    goto critical_error;
                }
            }

        bad_command:
            continue;
        }

    utf8_deinit(entry_str);
    return 0;

critical_error:
    utf8_deinit(entry_str);
    return -1;
}
