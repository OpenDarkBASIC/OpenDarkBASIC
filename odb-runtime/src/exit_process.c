#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
void odbrt_exit(int status)
{
    ExitProcess(status);
}
#else
#include <unistd.h>
void odbrt_exit(int status)
{
    _exit(status);
}
#endif
