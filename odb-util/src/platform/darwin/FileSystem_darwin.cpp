#include "odbc/util/FileSystem.hpp"
#include <unistd.h>

namespace odb {

// ----------------------------------------------------------------------------
FILE* dupFilePointer(FILE* file)
{
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
}

}
