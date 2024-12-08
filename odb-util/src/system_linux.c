#include "odb-util/system.h"
#include <unistd.h>

int
system_cpu_count(void)
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}
