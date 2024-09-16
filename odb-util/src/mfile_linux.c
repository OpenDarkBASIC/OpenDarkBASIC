#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/mfile.h"
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int
mfile_map_read(struct mfile* mf, struct ospathc filepath, int log_error)
{
    struct stat stbuf;
    int         fd;
    const char* c_file_name = ospathc_cstr(filepath);

    fd = open(c_file_name, O_RDONLY | O_LARGEFILE);
    if (fd < 0)
    {
        if (log_error)
            log_util_err(
                "Failed to open() file {quote:%s}: %s\n",
                c_file_name,
                strerror(errno));
        goto open_failed;
    }

    if (fstat(fd, &stbuf) != 0)
    {
        if (log_error)
            log_util_err(
                "Failed to fstat() file {quote:%s}: %s\n",
                c_file_name,
                strerror(errno));
        goto fstat_failed;
    }

    if (!S_ISREG(stbuf.st_mode))
    {
        if (log_error)
            log_util_err(
                "Cannot map file {quote:%s}: File is not a regular file\n",
                c_file_name);
        goto fstat_failed;
    }

    /*if (stbuf.st_size >= (1UL<<32))
        goto file_too_large;*/

    mf->address = mmap(
        NULL,
        (size_t)(stbuf.st_size),
        PROT_READ,
        MAP_PRIVATE | MAP_NORESERVE,
        fd,
        0);
    if (mf->address == MAP_FAILED)
    {
        if (log_error)
            log_util_err(
                "Failed to mmap() file {quote:%s}: %s\n",
                c_file_name,
                strerror(errno));
        goto mmap_failed;
    }

    /* file descriptor no longer required */
    close(fd);

    mem_track_allocation(mf->address);
    mf->size = (int)stbuf.st_size;
    return 0;

mmap_failed:
/*file_too_large: */
fstat_failed:
    close(fd);
open_failed:
    return -1;
}

int
mfile_map_overwrite(struct mfile* mf, int size, struct ospathc filepath)
{
    int         fd;
    const char* c_file_name = ospathc_cstr(filepath);

    fd = open(
        c_file_name,
        O_CREAT | O_RDWR | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        log_util_err(
            "Failed to open() file {quote:%s}: %s\n",
            c_file_name,
            strerror(errno));
        goto open_failed;
    }

    /* When truncating the file, it must be expanded again, otherwise writes to
     * the memory will cause SIGBUS.
     * NOTE: If this ever gets ported to non-Linux, see posix_fallocate() */
    if (fallocate(fd, 0, 0, size) != 0)
    {
        log_util_err(
            "Failed to resize file {quote:%s} to {quote:%d}: %s\n",
            c_file_name,
            size,
            strerror(errno));
        goto mmap_failed;
    }

    mf->address
        = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mf->address == MAP_FAILED)
    {
        log_util_err(
            "Failed to mmap() file {quote:%s} for writing: %s\n",
            c_file_name,
            strerror(errno));
        goto mmap_failed;
    }

    /* file descriptor no longer required */
    close(fd);

    mem_track_allocation(mf->address);
    mf->size = size;
    return 0;

mmap_failed:
    close(fd);
open_failed:
    return -1;
}

int
mfile_map_mem(struct mfile* mf, int size)
{
    mf->address = mmap(
        NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mf->address == MAP_FAILED)
    {
        log_util_err(
            "Failed to mmap() {emph:%d} bytes: %s\n", size, strerror(errno));
        return -1;
    }

    mem_track_allocation(mf->address);
    mf->size = size;
    return 0;
}

void
mfile_unmap(struct mfile* mf)
{
    mem_track_deallocation(mf->address);
    munmap(mf->address, (size_t)mf->size);
}
