#include "odb-compiler/semantic/semantic.h"

static int
check_loop_exit(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_exit = {check_loop_exit, depends};
