extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-util/log.h"
}

#include "LIEF/Abstract/Binary.hpp"
#include "LIEF/ELF.hpp"
#include "LIEF/PE.hpp"
#include <iostream>

static int
parse_command_string(
    struct cmd_list* commands,
    plugin_id        plugin_id,
    const char*      data,
    struct utf8_span str,
    struct ospathc   filepath)
{
    struct utf8_span cmd_name, type_str, c_symbol, db_params;
    utf8_split(data, str, '%', &cmd_name, &type_str);
    utf8_split(data, type_str, '%', &type_str, &c_symbol);
    utf8_split(data, c_symbol, '%', &c_symbol, &db_params);

    if (cmd_name.len == 0 || type_str.len == 0 || c_symbol.len == 0)
    {
        log_cmd_warn(
            "Invalid command string {quote:%.*s} in plugin {emph:%s}\n",
            str.len,
            data + str.off,
            ospathc_cstr(filepath));
        return 0;
    }

    enum type return_type = type_from_char(data[type_str.off]);
    if (return_type == 0)
    {
        log_cmd_warn(
            "Invalid command return type {quote:%c} in string {quote:%.*s} in "
            "plugin {emph:%s}\n",
            data[type_str.off],
            str.len,
            data + str.off,
            ospathc_cstr(filepath));
        return 0;
    }

    if (data[type_str.off + 1] != '(')
    {
        log_cmd_warn(
            "Expected {quote:(} after return type in command type string "
            "{quote:%.*s} in plugin {emph:%s}\n",
            str.len,
            data + str.off,
            ospathc_cstr(filepath));
        return 0;
    }

    /* Command names are case-insensitive. By convention we store them in upper
     * case in the command list */
    for (int c = 0; c != cmd_name.len; ++c)
        if (toupper(data[cmd_name.off + c]) != data[cmd_name.off + c])
        {
            log_cmd_warn(
                "Command names must be stored as lower case. Command string "
                "contains upper case characters {quote:%.*s} in plugin "
                "{emph:%s}\n",
                str.len,
                data + str.off,
                ospathc_cstr(filepath));
            return 0;
        }

    cmd_id cmd = cmd_list_add(
        commands,
        plugin_id,
        return_type,
        utf8_span_view(data, cmd_name),
        utf8_span_view(data, c_symbol));
    if (cmd < 0)
        goto critical_error;

    struct utf8_span db_param_name;
    for (utf8_idx i = 2; i < type_str.len - 1; ++i)
    {
        char                     type_char = data[type_str.off + i];
        enum cmd_param_direction direction = CMD_PARAM_IN;
        enum type                type = type_from_char(type_char);

        utf8_split(data, db_params, ',', &db_param_name, &db_params);

        if (type == 0)
        {
            log_cmd_warn(
                "Invalid command parameter type {quote:%c} in string "
                "{quote:%.*s} in plugin {emph:%s}\n",
                type_char,
                str.len,
                data + str.off,
                ospathc_cstr(filepath));
            /* Skip command, but plugin is still usable hopefully */
            cmd_list_erase(commands, cmd);
            return 0;
        }

        if (i + 1 < type_str.len && data[type_str.off + i + 1] == '*')
        {
            i++;
            direction = CMD_PARAM_OUT;
        }

        if (cmd_add_param(
                commands,
                cmd,
                type,
                direction,
                utf8_span_view(data, db_param_name))
            != 0)
        {
            goto critical_error;
        }
    }

    return 0;

critical_error:
    return -1;
}

static int
parse_string_section(
    const char*      data,
    struct utf8_span sec,
    struct ospathc   filepath,
    plugin_id        plugin_id,
    struct cmd_list* commands)
{
    struct utf8_span str;
    struct utf8_span next = sec;
    do
    {
        utf8_split(data, next, '\n', &str, &next);
        if (parse_command_string(commands, plugin_id, data, str, filepath) != 0)
            return -1;
    } while (next.len > 0);

    return 0;
}

static int
parse_string_table(
    const LIEF::PE::Binary* pe,
    struct ospathc          filepath,
    plugin_id               plugin_id,
    struct cmd_list*        commands)
{
    struct utf8 entry_str = empty_utf8();

    for (const auto& entry : pe->resources_manager().value().string_table())
    {
        const std::u16string& u16 = entry.name();
        struct utf16_view     u16v
            = {(const uint16_t*)u16.data(), (utf16_idx)u16.length()};
        if (utf16_to_utf8(&entry_str, u16v) != 0)
            goto fatal_error;

        if (parse_command_string(
                commands,
                plugin_id,
                entry_str.data,
                utf8_span(entry_str),
                filepath)
            != 0)
        {
            goto fatal_error;
        }
    }

    utf8_deinit(entry_str);
    return 0;

fatal_error:
    utf8_deinit(entry_str);
    return -1;
}

int
load_odb_commands(
    struct cmd_list*    commands,
    plugin_id           plugin_id,
    const LIEF::Binary* binary,
    struct ospathc      filepath)
{
    switch (binary->format())
    {
        case LIEF::Binary::ELF: {
            auto elf = static_cast<const LIEF::ELF::Binary*>(binary);
            const LIEF::Section* odbres = elf->get_section(".odbres");
            if (odbres == nullptr)
                return log_cmd_err(
                    "Missing .odbres section in plugin {emph:%s}.\n",
                    ospathc_cstr(filepath));

            auto             content = odbres->content();
            struct utf8_span span = {0, (utf8_idx)content.size()};
            return parse_string_section(
                (const char*)content.data(),
                span,
                filepath,
                plugin_id,
                commands);
        }

        case LIEF::Binary::PE: {
            auto pe = static_cast<const LIEF::PE::Binary*>(binary);

            if (pe->resources() == nullptr)
            {
                log_cmd_warn(
                    "No resources found in pluign {emph:%s}.\n",
                    ospathc_cstr(filepath));
                return 0;
            }

            return parse_string_table(pe, filepath, plugin_id, commands);
        }

        case LIEF::Binary::UNKNOWN:
        case LIEF::Binary::MACHO:
        case LIEF::Binary::OAT:
        default:
            log_cmd_err(
                "Loading plugin format %d is not yet supported\n",
                binary->format());
            break;
    }

    return 0;
}
