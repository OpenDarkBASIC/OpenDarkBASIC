#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
int odbrt_add_dll_path(const char* path)
{
    if (!SetDllDirectoryA(path))
        return -1;
    return 0;
}
#else
int odbrt_add_dll_path(const char* path)
{
    return 0;
}
#endif
