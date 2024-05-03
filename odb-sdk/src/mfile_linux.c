#define _LARGEFILE64_SOURCE
#include "odb-sdk/log.h"
#include "odb-sdk/mfile.h"
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int
mfile_map_cow_with_extra_padding(struct mfile* mf, struct ospath_view file, int padding)
{
    struct stat stbuf;
    int         fd;
    const char* c_file_name = ospath_view_cstr(file);

    fd = open(c_file_name, O_RDONLY | O_LARGEFILE);
    if (fd < 0)
    {
        log_sdk_err(
            "Failed to open() file {quote:%s}: %s\n",
            c_file_name,
            strerror(errno));
        goto open_failed;
    }

    if (fstat(fd, &stbuf) != 0)
    {
        log_sdk_err(
            "Failed to fstat() file {quote:%s}: %s\n",
            c_file_name,
            strerror(errno));
        goto fstat_failed;
    }

    if (!S_ISREG(stbuf.st_mode))
    {
        log_sdk_err(
            "Cannot map file {quote:%s}: File is not a regular file\n",
            c_file_name);
        goto fstat_failed;
    }

    /*if (stbuf.st_size >= (1UL<<32))
        goto file_too_large;*/

    mf->address = mmap(
        NULL,
        (size_t)(stbuf.st_size + padding),
        PROT_READ,
        MAP_PRIVATE | MAP_NORESERVE,
        fd,
        0);
    if (mf->address == MAP_FAILED)
    {
        log_sdk_err(
            "Failed to mmap() file {quote:%s}: %s\n",
            c_file_name,
            strerror(errno));
        goto mmap_failed;
    }

    /* file descriptor no longer required */
    close(fd);

    mf->size = (int)(stbuf.st_size + padding);
    return 0;

mmap_failed:
/*file_too_large :*/
fstat_failed:
    close(fd);
open_failed:
    return -1;
}

void
mfile_unmap(struct mfile* mf)
{
    munmap(mf->address, (size_t)mf->size);
}
