#include "odb-util/system.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int
system_cpu_count(void)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (int)si.dwNumberOfProcessors;
}
