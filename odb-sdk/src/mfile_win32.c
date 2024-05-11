#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-sdk/mem.h"
#include "odb-sdk/mfile.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/log.h"

int
mfile_map_cow_with_extra_padding(struct mfile* mf, struct ospath_view filepath, int padding)
{
    HANDLE hFile;
    LARGE_INTEGER liFileSize;
    HANDLE mapping;
    struct utf16 utf16_filename = empty_utf16();
    DWORD map_size;

    if (utf8_to_utf16(&utf16_filename, filepath.str) != 0)
        goto utf16_conv_failed;

    /* Try to open the file */
    hFile = CreateFileW(
        utf16_cstr(utf16_filename), /* File name */
        GENERIC_READ,           /* Read only */
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,                   /* Default security */
        OPEN_EXISTING,          /* File must exist */
        FILE_ATTRIBUTE_NORMAL,  /* Default attributes */
        NULL);                  /* No attribute template */
    if (hFile == INVALID_HANDLE_VALUE)
        goto open_failed;

    /* Determine file size in bytes */
    if (!GetFileSizeEx(hFile, &liFileSize))
        goto get_file_size_failed;
    liFileSize.QuadPart += padding;
    if (liFileSize.QuadPart > (1ULL << 32) - 1)  /* mf->size is an int */
    {
        log_sdk_err(
            "Failed to map file {quote:%s}: Mapping files >4GiB is not implemented\n",
            ospath_view_cstr(filepath));
        goto get_file_size_failed;
    }
    map_size = liFileSize.LowPart;

    mapping = CreateFileMapping(
        hFile,                 /* File handle */
        NULL,                  /* Default security attributes */
        PAGE_READONLY,         /* Read only (or copy on write, but we don't write) */
        0, map_size,           /* High/Low size of mapping. Zero means entire file */
        NULL);                 /* Don't name the mapping */
    if (mapping == NULL)
        goto create_file_mapping_failed;

    mf->address = MapViewOfFile(
        mapping,               /* File mapping handle */
        FILE_MAP_COPY,         /* Copy-on-Write */
        0, 0,                  /* High/Low offset of where the mapping should begin in the file */
        map_size);             /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
        goto map_view_failed;

    /* The file mapping isn't required anymore */
    CloseHandle(mapping);
    CloseHandle(hFile);
    utf16_deinit(utf16_filename);

    mem_track_allocation(mf->address);
    mf->size = map_size;

    return 0;

    map_view_failed            : CloseHandle(mapping);
    create_file_mapping_failed :
    get_file_size_failed       : CloseHandle(hFile);
    open_failed                : utf16_deinit(utf16_filename);
    utf16_conv_failed          : return -1;
}

int
mfile_map_mem(struct mfile* mf, int size)
{
    HANDLE mapping = CreateFileMapping(
        INVALID_HANDLE_VALUE,  /* File handle */
        NULL,                  /* Default security attributes */
        PAGE_READWRITE,        /* Read + Write access */
        0, size,               /* High/Low size of mapping. Zero means entire file */
        NULL);                 /* Don't name the mapping */
    if (mapping == NULL)
        goto create_file_mapping_failed;

    mf->address = MapViewOfFile(
        mapping,               /* File mapping handle */
        FILE_MAP_WRITE,        /* Read + Write */
        0, 0,                  /* High/Low offset of where the mapping should begin in the file */
        size);                 /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
        goto map_view_failed;

    CloseHandle(mapping);

    mem_track_allocation(mf->address);
    mf->size = size;

    return 0;

    map_view_failed            : CloseHandle(mapping);
    create_file_mapping_failed : return -1;
}

void mfile_unmap(struct mfile* mf)
{
    mem_track_deallocation(mf->address);
    UnmapViewOfFile(mf->address);
}
