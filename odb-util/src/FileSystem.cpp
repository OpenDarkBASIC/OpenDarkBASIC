#include "odbc/util/FileSystem.hpp"

#if defined(ODBC_PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#elif defined(ODBC_PLATFORM_MACOS) || defined(ODBC_PLATFORM_LINUX)
#include <unistd.h>
#endif

namespace odbc {

// ----------------------------------------------------------------------------
FILE* dupFilePointer(FILE* file)
{
#if defined(ODBC_PLATFORM_WIN32)
    FILE* newFp;
    int fd = _fileno(file);
    int newFd = _dup(fd);
    if (newFd == -1)
        goto dup_failed;
    newFp = _fdopen(newFd, "r");
    if (newFp == nullptr)
        goto fdopen_failed;

    return newFp;

    fdopen_failed : _close(newFd);
    dup_failed    : return nullptr;
#elif defined(ODBC_PLATFORM_MACOS) || defined(ODBC_PLATFORM_LINUX)
    FILE* newFp;
    int fd = fileno(file);
    int newFd = dup(fd);
    if (newFd == -1)
        goto dup_failed;
    newFp = fdopen(newFd, "r");
    if (newFp == nullptr)
        goto fdopen_failed;

    return newFp;

    fdopen_failed : close(newFd);
    dup_failed    : return nullptr;
#endif
}

}
