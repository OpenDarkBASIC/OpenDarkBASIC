#include "odbc/util/FileSystem.hpp"
#include <Windows.h>
#include <io.h>

namespace odbc {

// ----------------------------------------------------------------------------
FILE* dupFilePointer(FILE* file)
{
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
}

}
