#include "core-commands/config.h"
#include <unistd.h>

ODBPLUGIN_API void exit(int status)
{
    _exit(status);
}
