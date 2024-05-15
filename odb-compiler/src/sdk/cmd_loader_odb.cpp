extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/log.h"
}

#include "LIEF/Abstract/Binary.hpp"
#include "LIEF/ELF.hpp"

int
load_odb_commands(
    struct cmd_list*    commands,
    plugin_ref          plugin_id,
    const LIEF::Binary* binary,
    struct ospathc      filepath)
{
    if (binary->format() == LIEF::Binary::FORMATS::ELF)
    {
        auto elf = static_cast<const LIEF::ELF::Binary*>(binary);
        for (const auto& sym : elf->symbols())
        {
            log_dbg(
                "",
                "%s: %lu, %lu\n",
                sym.name().c_str(),
                sym.value(),
                elf->virtual_address_to_offset(sym.value()).value());
        }
        for (const auto& str : elf->strings())
            log_dbg("", "str: %s\n", str.c_str());
        log_dbg("", "%s\n", elf->get_dynamic_symbol("test_command_help")->name().c_str());
    }
    return -1;
}
