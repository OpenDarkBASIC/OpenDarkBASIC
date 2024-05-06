#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/command.h"
#include "odb-compiler/sdk/sdk.h"
#include "odb-sdk/vec.h"

struct plugin_list;

VEC_DECLARE_API(command_list, struct command, 32)

ODBCOMPILER_PUBLIC_API int
commands_load_all(
    struct command_list*      commands,
    enum sdk_type             sdk_type,
    const struct plugin_list* plugins);
