#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/command.h"
#include "odb-compiler/sdk/sdk.h"
#include "odb-sdk/vec.h"
#include "odb-sdk/utf8_list.h"

struct plugin_list;

struct command_list
{
    struct utf8_list db_identifiers;
    struct utf8_list cpp_identifiers;
    struct utf8_list help_files;
    struct v1616 plugin_refs;
};

ODBCOMPILER_PUBLIC_API int
commands_load_all(
    struct command_list*      commands,
    enum sdk_type             sdk_type,
    const struct plugin_list* plugins);
