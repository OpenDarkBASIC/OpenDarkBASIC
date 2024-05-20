#pragma once

extern "C" {
#include "odb-compiler/parser/db_parser.h"
#include "odb-compiler/sdk/cmd_list.h"
}

class CommandListHelper
{
public:
    CommandListHelper();
    ~CommandListHelper();

    int
    addCommand(const char* name);
    int
    addCommand(enum cmd_arg_type return_type, const char* name);

};
