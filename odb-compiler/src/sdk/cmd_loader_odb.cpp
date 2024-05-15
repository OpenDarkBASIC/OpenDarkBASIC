extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/log.h"
}

#include "LIEF/Abstract/Binary.hpp"
#include "LIEF/ELF.hpp"
#include <iostream>

int
load_odb_commands(
    struct cmd_list*    commands,
    plugin_ref          plugin_id,
    const LIEF::Binary* binary,
    struct ospathc      filepath)
{
    switch (binary->format())
    {
        case LIEF::Binary::FORMATS::ELF: {
            auto elf = static_cast<const LIEF::ELF::Binary*>(binary);
            for (const auto& sym : elf->symbols())
            {
                log_dbg(
                    "",
                    "%s: %lu, %lu\n",
                    sym.name().c_str(),
                    sym.size(),
                    elf->virtual_address_to_offset(sym.value()).value());
                for (auto c : elf->get_content_from_virtual_address(
                         sym.value(), sym.size()))
                    log_raw("%02x ", (int)c);
                log_raw("\n");
            }
            for (const LIEF::Symbol& s : elf->dynamic_symbols())
                std::cout << s << '\n';

            for (const auto& str : elf->strings())
                log_dbg("", "str: %s\n", str.c_str());
            log_dbg(
                "",
                "%s\n",
                elf->get_dynamic_symbol("test_command_help")->name().c_str());
            break;
        }

        case LIEF::Binary::UNKNOWN:
        case LIEF::Binary::PE:
        case LIEF::Binary::MACHO:
        case LIEF::Binary::OAT:
            log_sdk_err(
                "Loading plugin format %d is not yet supported\n",
                binary->format());
            return -1;
    }

    return 0;
}
