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
        case LIEF::Binary::ELF:
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
