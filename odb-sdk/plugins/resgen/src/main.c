#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define NL "\r\n"
#else
#define _GNU_SOURCE
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#define NL "\n"
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------------------
 * Platform abstractions & Utilities
 * ------------------------------------------------------------------------- */

/*! Memory-mapped file */
struct mfile
{
    void* address;
    int   size;
};

#if defined(WIN32)
static void
print_last_win32_error(void)
{
    LPSTR  msg;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msg,
        0,
        NULL);
    fprintf(stderr, "Error: %.*s", (int)size, msg);
    LocalFree(msg);
}
static wchar_t*
utf8_to_utf16(const char* utf8, int utf8_bytes)
{
    int utf16_bytes
        = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_bytes, NULL, 0);
    if (utf16_bytes == 0)
        return NULL;

    wchar_t* utf16 = malloc((sizeof(wchar_t) + 1) * utf16_bytes);
    if (utf16 == NULL)
        return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, utf8, utf8_bytes, utf16, utf16_bytes)
        == 0)
    {
        free(utf16);
        return NULL;
    }

    utf16[utf16_bytes] = 0;

    return utf16;
}
static void
utf_free(void* utf)
{
    free(utf);
}
#endif

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
            fprintf(
                stderr,
                "Error: Failed to open file \"%s\": %s\n",
                file_path,
                strerror(errno));
        goto open_failed;
    }

    if (fstat(fd, &stbuf) != 0)
    {
        fprintf(
            stderr,
            "Error: Failed to stat file \"%s\": %s\n",
            file_path,
            strerror(errno));
        goto fstat_failed;
    }
    if (!S_ISREG(stbuf.st_mode))
    {
        fprintf(
            stderr, "Error: File \"%s\" is not a regular file!\n", file_path);
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
        fprintf(
            stderr,
            "Error: Failed to mmap() file \"%s\": %s\n",
            file_path,
            strerror(errno));
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
        fprintf(
            stderr,
            "Error: Failed to open file \"%s\" for writing: %s\n",
            file_path,
            strerror(errno));
        goto open_failed;
    }

    /* When truncating the file, it must be expanded again, otherwise writes to
     * the memory will cause SIGBUS.
     * NOTE: If this ever gets ported to non-Linux, see posix_fallocate() */
    if (fallocate(fd, 0, 0, size) != 0)
    {
        fprintf(
            stderr,
            "Error: Failed to resize file \"%s\": %s\n",
            file_path,
            strerror(errno));
        goto mmap_failed;
    }

    mf->address
        = mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mf->address == MAP_FAILED)
    {
        fprintf(
            stderr,
            "Error: Failed to mmap() file \"%s\" for writing: %s\n",
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

/*! All strings are represented as an offset and a length into a buffer. */
struct str_view
{
    int off, len;
};

/*! Create a string view from a c-string */
static struct str_view
str_view(const char* cstr)
{
    struct str_view str;
    str.off = 0;
    str.len = strlen(cstr);
    return str;
}
/*!
 * \brief Checks for equality between a c-string and a string view.
 * \param[in] s1 C-String to compare.
 * \param[in] s2 String view to compare.
 * \param[in] data Pointer to buffer containing the data of the string view.
 * \return Return non-zero if equal, zero if the strings differ.
 */
static int
cstr_eq_str(const char* s1, struct str_view s2, const char* data)
{
    int len = (int)strlen(s1);
    return len == s2.len && memcmp(data + s2.off, s1, len) == 0;
}
/*!
 * \brief Checks for equality between two string views.
 * \param[in] s1 First string view.
 * \param[in] s2 Second string view.
 * \param[in] data Pointer to buffer containing the data of both string views.
 * \return Return non-zero if equal, zero if the strings differ.
 */
static int
str_eq_str(struct str_view s1, struct str_view s2, const char* data)
{
    return s1.len == s2.len
           && memcmp(data + s1.off, data + s2.off, s1.len) == 0;
}
/*!
 * \brief Convert a string view to a decimal integer.
 * \param[in] str String view to convert.
 * \param[in] dataPointer to buffer containing the data of the string view.
 * \return Return the converted value, or 0 if an error occurred.
 */
static int
str_dec_to_int(struct str_view str, const char* data)
{
    int i;
    int value = 0;
    for (i = 0; i != str.len; ++i)
    {
        char b = data[str.off + i];
        if (b >= '0' && b <= '9')
            value = value * 10 + (b - '0');
        else
            return 0;
    }

    return value;
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
mstream_pad(struct mstream* ms, int additional_size)
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
    mstream_pad(ms, 1);
    ((char*)ms->address)[ms->write_ptr++] = c;
}

/*! Convert an integer to a string and write it to the mstream buffer */
static inline void
mstream_write_int(struct mstream* ms, int value)
{
    int digit = 1000000000;
    mstream_pad(ms, sizeof("-2147483648") - 1);
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
    mstream_pad(ms, len);
    memcpy((char*)ms->address + ms->write_ptr, cstr, len);
    ms->write_ptr += len;
}

/*! Write a string view to the mstream buffer */
static inline void
mstream_str(struct mstream* ms, struct str_view str, const char* data)
{
    mstream_pad(ms, str.len);
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
                case 's': mstream_cstr(ms, va_arg(va, const char*)); continue;
                case 'i':
                case 'd': mstream_write_int(ms, va_arg(va, int)); continue;
                case 'S': {
                    struct str_view str = va_arg(va, struct str_view);
                    const char*     data = va_arg(va, const char*);
                    mstream_str(ms, str, data);
                }
                    continue;
            }
        mstream_putc(ms, fmt[i]);
    }
    va_end(va);
}

/* ----------------------------------------------------------------------------
 * Settings & Command Line
 * ------------------------------------------------------------------------- */

enum target
{
    TARGET_NONE = 0,
    TARGET_WINRES,
    TARGET_ELF
};

struct cfg
{
    const char* output_file;
    char**      input_files;
    int         input_files_count;
    enum target target;
};

static int
parse_cmdline(int argc, char** argv, struct cfg* cfg)
{
    int i;
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: Missing argument to option -i\n");
                return -1;
            }

            cfg->input_files_count = 0;
            cfg->input_files = &argv[i + 1];
            for (i++; i != argc; i++)
            {
                if (argv[i][0] == '-')
                {
                    i--;
                    break;
                }
                cfg->input_files_count++;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: Missing argument to option -o\n");
                return -1;
            }

            cfg->output_file = argv[++i];
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            const char* target_name;
            if (i + 1 >= argc)
            {
                fprintf(stderr, "Error: Missing argument to option -t\n");
                return -1;
            }

            target_name = argv[++i];
            if (strcmp(target_name, "winres") == 0)
                cfg->target = TARGET_WINRES;
            else if (strcmp(target_name, "elf") == 0)
                cfg->target = TARGET_ELF;
            else
            {
                fprintf(stderr, "Unknown target '%s'\n", target_name);
                return -1;
            }
        }
    }

    if (cfg->target == TARGET_NONE)
    {
        fprintf(
            stderr,
            "Error: No target was specified. Use -t <winres|elf> to specify "
            "the output format\n");
        return -1;
    }

    if (cfg->input_files == NULL || cfg->input_files_count == 0)
    {
        fprintf(stderr, "Error: No input files specified. Use -i <files...>\n");
        return -1;
    }

    if (cfg->output_file == NULL)
    {
        fprintf(stderr, "Error: No output file specified. Use -o <output>\n");
        return -1;
    }

    return 0;
}

/* ----------------------------------------------------------------------------
 * Parser
 * ------------------------------------------------------------------------- */

struct parser
{
    const char* data;
    int         tail;
    int         head;
    int         len;
    union
    {
        struct str_view str;
        int             integer;
    } value;
};

static void
parser_init(struct parser* p, struct mfile* mf)
{
    p->data = (char*)mf->address;
    p->head = 0;
    p->tail = 0;
    p->len = mf->size;
}

enum token
{
    TOK_ERROR = -1,
    TOK_END = 0,
    TOK_ODB_COMMAND = 256,
    TOK_STRING,
};

static int
print_error(struct parser* p, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    fprintf(stderr, "%.*s\n", p->head - p->tail, p->data + p->tail);
    return -1;
}

enum token
scan_next_token(struct parser* p)
{
    p->tail = p->head;
    while (p->head != p->len)
    {
        /* ".*?" */
        if (p->data[p->head] == '"')
        {
            p->value.str.off = ++p->head;
            for (; p->head != p->len; ++p->head)
                if (p->data[p->head] == '"')
                    break;
            if (p->head == p->len)
                return print_error(
                    p, "Error: Missing closing quote on string\n");
            p->value.str.len = p->head++ - p->value.str.off;
            return TOK_STRING;
        }
        if (memcmp(p->data + p->head, "ODB_COMMAND", sizeof("ODB_COMMAND") - 1)
            == 0)
        {
            while (p->head != p->len && isdigit(p->data[p->head]))
                p->head++;
            return TOK_ODB_COMMAND;
        }
    }

    return TOK_END;
}

static int
parse(struct parser* p, const struct cfg* cfg)
{
    enum token tok;
    do
    {
        switch ((tok = scan_next_token(p)))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;
            case TOK_ODB_COMMAND: break;
            case TOK_STRING: break;
        }
    } while (1);
}

static int
gen_resource(const char* filename)
{
    struct mfile   mf;
    struct mstream ms = mstream_init_writeable();

    /* Don't write resource if it is identical to the existing one -- causes
     * less rebuilds */
    if (mfile_map_read(&mf, filename, 1) == 0)
    {
        if (mf.size == ms.write_ptr
            && memcmp(mf.address, ms.address, mf.size) == 0)
            return 0;
        mfile_unmap(&mf);
    }

    if (mfile_map_write(&mf, filename, ms.write_ptr) != 0)
        return -1;
    memcpy(mf.address, ms.address, ms.write_ptr);
    mfile_unmap(&mf);

    return 0;
}

int
main(int argc, char** argv)
{
    struct cfg cfg = {0};
    if (parse_cmdline(argc, argv, &cfg) != 0)
        return -1;

    return 0;
}
