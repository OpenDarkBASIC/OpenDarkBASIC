#if defined(_WIN32)
#define NL "\r\n"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define NL "\n"
#define _GNU_SOURCE
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ----------------------------------------------------------------------------
 * Printing and formatting
 * ------------------------------------------------------------------------- */

/*! All strings are represented as an offset and a length into a buffer. */
struct str_view
{
    int off, len;
};

static int disable_colors = 0;

static const char*
emph_style(void)
{
    return disable_colors ? "" : "\033[1;37m";
}
static const char*
error_style(void)
{
    return disable_colors ? "" : "\033[1;31m";
}
static const char*
underline_style(void)
{
    return disable_colors ? "" : "\033[1;31m";
}
static const char*
reset_style(void)
{
    return disable_colors ? "" : "\033[0m";
}

static int
print_error(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "%serror:%s ", error_style(), reset_style());
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    return -1;
}

/* ----------------------------------------------------------------------------
 * Platform abstractions & Utilities
 * ------------------------------------------------------------------------- */

/*! Used to disable colors if the stream is being redirected */
#if defined(WIN32)
static int
stream_is_terminal(FILE* fp)
{
    return 1;
}
#else
static int
stream_is_terminal(FILE* fp)
{
    return isatty(fileno(fp));
}
#endif

/*! Memory-mapped file */
struct mfile
{
    void* address;
    int   size;
};

/*!
 * \brief Memory-maps a file in read-only mode.
 * \param[in] mf Pointer to mfile structure. Struct can be uninitialized.
 * \param[in] file_path Utf8 encoded file path.
 * \param[in] silence_open_error If zero, an error is printed to stderr if
 * mapping fails. If one, no errors are printed. When comparing the generated
 * code with an already existing file, we allow the function to fail silently if
 * the file does not exist. \return Returns 0 on success, negative on failure.
 */
static int
mfile_map_read(struct mfile* mf, const char* file_path, int silence_open_error)
{
#if defined(WIN32)
    HANDLE        hFile;
    LARGE_INTEGER liFileSize;
    HANDLE        mapping;
    wchar_t*      utf16_file_path;

    utf16_file_path = utf8_to_utf16(file_path, (int)strlen(file_path));
    if (utf16_file_path == NULL)
        goto utf16_conv_failed;

    /* Try to open the file */
    hFile = CreateFileW(
        utf16_file_path, /* File name */
        GENERIC_READ,    /* Read only */
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,                  /* Default security */
        OPEN_EXISTING,         /* File must exist */
        FILE_ATTRIBUTE_NORMAL, /* Default attributes */
        NULL);                 /* No attribute template */
    if (hFile == INVALID_HANDLE_VALUE)
        goto open_failed;

    /* Determine file size in bytes */
    if (!GetFileSizeEx(hFile, &liFileSize))
        goto get_file_size_failed;
    if (liFileSize.QuadPart > (1ULL << 32) - 1) /* mf->size is an int */
        goto get_file_size_failed;

    mapping = CreateFileMapping(
        hFile,         /* File handle */
        NULL,          /* Default security attributes */
        PAGE_READONLY, /* Read only (or copy on write, but we don't write) */
        0,
        0,     /* High/Low size of mapping. Zero means entire file */
        NULL); /* Don't name the mapping */
    if (mapping == NULL)
        goto create_file_mapping_failed;

    mf->address = MapViewOfFile(
        mapping,       /* File mapping handle */
        FILE_MAP_READ, /* Read-only view of file */
        0,
        0,  /* High/Low offset of where the mapping should begin in the file */
        0); /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
        goto map_view_failed;

    /* The file mapping isn't required anymore */
    CloseHandle(mapping);
    CloseHandle(hFile);
    utf_free(utf16_file_path);

    mf->size = (int)liFileSize.QuadPart;

    return 0;

map_view_failed:
    CloseHandle(mapping);
create_file_mapping_failed:
get_file_size_failed:
    CloseHandle(hFile);
open_failed:
    utf_free(utf16_file_path);
utf16_conv_failed:
    return -1;
#else
    struct stat stbuf;
    int         fd;

    fd = open(file_path, O_RDONLY);
    if (fd < 0)
    {
        if (!silence_open_error)
            print_error(
                "Failed to open file \"%s\": %s\n", file_path, strerror(errno));
        goto open_failed;
    }

    if (fstat(fd, &stbuf) != 0)
    {
        print_error(
            "Failed to stat file \"%s\": %s\n", file_path, strerror(errno));
        goto fstat_failed;
    }
    if (!S_ISREG(stbuf.st_mode))
    {
        print_error("File \"%s\" is not a regular file!\n", file_path);
        goto fstat_failed;
    }

    mf->address = mmap(
        NULL,
        (size_t)stbuf.st_size,
        PROT_READ,
        MAP_PRIVATE | MAP_NORESERVE,
        fd,
        0);
    if (mf->address == MAP_FAILED)
    {
        print_error(
            "Failed to mmap() file \"%s\": %s\n", file_path, strerror(errno));
        goto mmap_failed;
    }

    /* file descriptor no longer required */
    close(fd);

    mf->size = (int)stbuf.st_size;
    return 0;

mmap_failed:
fstat_failed:
    close(fd);
open_failed:
    return -1;
#endif
}

/*!
 * \brief Memory-maps a file in read-write mode.
 * \param[in] mf Pointer to mfile structure. Struct can be uninitialized.
 * \param[in] file_path Utf8 encoded file path.
 * \param[in] size Size of the file in bytes. This is used to allocate space
 * on the file system.
 * \return Returns 0 on success, negative on failure.
 */
static int
mfile_map_write(struct mfile* mf, const char* file_path, int size)
{
#if defined(WIN32)
    HANDLE   hFile;
    HANDLE   mapping;
    wchar_t* utf16_file_path;

    utf16_file_path = utf8_to_utf16(file_path, (int)strlen(file_path));
    if (utf16_file_path == NULL)
        goto utf16_conv_failed;

    /* Try to open the file */
    hFile = CreateFileW(
        utf16_file_path,              /* File name */
        GENERIC_READ | GENERIC_WRITE, /* Read/write */
        0,
        NULL,                  /* Default security */
        CREATE_ALWAYS,         /* Overwrite any existing, otherwise create */
        FILE_ATTRIBUTE_NORMAL, /* Default attributes */
        NULL);                 /* No attribute template */
    if (hFile == INVALID_HANDLE_VALUE)
    {
        print_last_win32_error();
        goto open_failed;
    }

    mapping = CreateFileMappingW(
        hFile,          /* File handle */
        NULL,           /* Default security attributes */
        PAGE_READWRITE, /* Read + Write */
        0,
        size,  /* High/Low size of mapping */
        NULL); /* Don't name the mapping */
    if (mapping == NULL)
        goto create_file_mapping_failed;

    mf->address = MapViewOfFile(
        mapping,                        /* File mapping handle */
        FILE_MAP_READ | FILE_MAP_WRITE, /* Read + Write */
        0,
        0,  /* High/Low offset of where the mapping should begin in the file */
        0); /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
        goto map_view_failed;

    /* The file mapping isn't required anymore */
    CloseHandle(mapping);
    CloseHandle(hFile);
    utf_free(utf16_file_path);

    mf->size = size;

    return 0;

map_view_failed:
    CloseHandle(mapping);
create_file_mapping_failed:
    CloseHandle(hFile);
open_failed:
    utf_free(utf16_file_path);
utf16_conv_failed:
    return -1;
#else
    int fd = open(
        file_path,
        O_CREAT | O_RDWR | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        print_error(
            "Failed to open file \"%s\" for writing: %s\n",
            file_path,
            strerror(errno));
        goto open_failed;
    }

    /* When truncating the file, it must be expanded again, otherwise writes to
     * the memory will cause SIGBUS.
     * NOTE: If this ever gets ported to non-Linux, see posix_fallocate() */
    if (fallocate(fd, 0, 0, size) != 0)
    {
        print_error(
            "Failed to resize file \"%s\" to %d: %s\n",
            file_path,
            size,
            strerror(errno));
        goto mmap_failed;
    }

    mf->address
        = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mf->address == MAP_FAILED)
    {
        print_error(
            "Failed to mmap() file \"%s\" for writing: %s\n",
            file_path,
            strerror(errno));
        goto mmap_failed;
    }

    /* file descriptor no longer required */
    close(fd);

    mf->size = size;
    return 0;

mmap_failed:
    close(fd);
open_failed:
    return -1;
#endif
}

/*!
 * \brief Unmaps a previously memory-mapped file.
 * \param mf Pointer to mfile structure.
 */
void
mfile_unmap(struct mfile* mf)
{
#if defined(WIN32)
    UnmapViewOfFile(mf->address);
#else
    munmap(mf->address, (size_t)mf->size);
#endif
}


/*! A memory buffer that grows as data is added. */
struct mstream
{
    void* address;
    int   capacity;
    int   write_ptr;
};

/*! Init a new mstream structure ready for writing */
static struct mstream
mstream_init_writeable(void)
{
    struct mstream ms;
    ms.address = NULL;
    ms.capacity = 0;
    ms.write_ptr = 0;
    return ms;
}

/*!
 * \brief Grows the capacity of mstream if required
 * \param[in] ms Memory stream structure.
 * \param[in] additional_size How many bytes to grow the capacity of the buffer
 * by.
 */
static inline void
mstream_grow(struct mstream* ms, int additional_size)
{
    while (ms->capacity < ms->write_ptr + additional_size)
    {
        ms->capacity = ms->capacity == 0 ? 32 : ms->capacity * 2;
        ms->address = realloc(ms->address, ms->capacity);
    }
}

/*! Write a single character (byte) to the mstream buffer */
static inline void
mstream_putc(struct mstream* ms, char c)
{
    mstream_grow(ms, 1);
    ((char*)ms->address)[ms->write_ptr++] = c;
}

/*! Convert an integer to a string and write it to the mstream buffer */
static inline void
mstream_write_int(struct mstream* ms, int value)
{
    int digit = 1000000000;
    mstream_grow(ms, sizeof("-2147483648") - 1);
    if (value < 0)
    {
        ((char*)ms->address)[ms->write_ptr++] = '-';
        value = -value;
    }
    else if (value == 0)
    {
        ((char*)ms->address)[ms->write_ptr++] = '0';
        return;
    }
    while (value < digit)
        digit /= 10;
    while (digit)
    {
        ((char*)ms->address)[ms->write_ptr] = '0';
        while (value >= digit)
        {
            value -= digit;
            ((char*)ms->address)[ms->write_ptr]++;
        }
        ms->write_ptr++;
        digit /= 10;
    }
}

/*! Write a C-string to the mstream buffer */
static inline void
mstream_cstr(struct mstream* ms, const char* cstr)
{
    int len = (int)strlen(cstr);
    mstream_grow(ms, len);
    memcpy((char*)ms->address + ms->write_ptr, cstr, len);
    ms->write_ptr += len;
}

/*! Write a string view to the mstream buffer */
static inline void
mstream_str(struct mstream* ms, struct str_view str, const char* data)
{
    mstream_grow(ms, str.len);
    memcpy((char*)ms->address + ms->write_ptr, data + str.off, str.len);
    ms->write_ptr += str.len;
}

/*!
 * \brief Write a formatted string to the mstream buffer.
 * This function is similar to printf() but only implements a subset of the
 * format specifiers. These are:
 *   %i - Write an integer (int)
 *   %d - Write an integer (int)
 *   %s - Write a c-string (const char*)
 *   %S - Write a string view (struct str_view, const char*)
 * \param[in] ms Pointer to mstream structure.
 * \param[in] fmt Format string.
 * \param[in] ... Additional parameters.
 */
static inline void
mstream_fmt(struct mstream* ms, const char* fmt, ...)
{
    int     i;
    va_list va;
    va_start(va, fmt);
    for (i = 0; fmt[i]; ++i)
    {
        if (fmt[i] == '%')
            switch (fmt[++i])
            {
                case 'c': mstream_putc(ms, (char)va_arg(va, int)); continue;
                case 's': mstream_cstr(ms, va_arg(va, const char*)); continue;
                case 'i':
                case 'd': mstream_write_int(ms, va_arg(va, int)); continue;
                case 'S': {
                    struct str_view str = va_arg(va, struct str_view);
                    const char*     data = va_arg(va, const char*);
                    mstream_str(ms, str, data);
                    continue;
                }
            }
        mstream_putc(ms, fmt[i]);
    }
    va_end(va);
}


static void
write_preamble(struct mstream* ms)
{
    mstream_cstr(ms, "#include <gmock/gmock.h>\n\n");
    mstream_cstr(ms, "extern \"C\" {\n");
    mstream_cstr(ms, "#include <gmock/gmock.h>\n");
    mstream_cstr(ms, "}\n\n");

    mstream_cstr(ms, "#define NAME odbcompiler_ci\n\n");

    mstream_cstr(ms, "using namespace testing;\n\n");

    mstream_cstr(ms, "struct NAME : Test\n");
    mstream_cstr(ms, "{\n");
    mstream_cstr(ms, "};\n\n");
}

int
main(int argc, char** argv)
{
    if (!stream_is_terminal(stderr))
        disable_colors = 1;

    print_error("Hello, world!\n");
    return -1;
}
