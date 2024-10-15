#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define _GNU_SOURCE
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------------------
 * Printing and formatting
 * ------------------------------------------------------------------------- */

/*! All strings are represented as an offset and a length into a buffer. */
struct str_view
{
    int off, len;
};

static struct str_view
empty_str_view(void)
{
    struct str_view str = {0, 0};
    return str;
}

static int
str_equal(struct str_view s1, struct str_view s2, const char* data)
{
    if (s1.len != s2.len)
        return 0;
    return memcmp(data + s1.off, data + s2.off, s1.len) == 0;
}

static int
cstr_equal(const char* s1, struct str_view s2, const char* data)
{
    if ((int)strlen(s1) != s2.len)
        return 0;
    return memcmp(s1, data + s2.off, s2.len) == 0;
}

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

static void
log_vflc(
    const char*     filename,
    const char*     source,
    struct str_view loc,
    const char*     fmt,
    va_list         ap)
{
    int i;
    int l1, c1;

    l1 = 1, c1 = 1;
    for (i = 0; i != loc.off; i++)
    {
        c1++;
        if (source[i] == '\n')
            l1++, c1 = 1;
    }

    fprintf(
        stderr,
        "%s%s:%d:%d:%s ",
        emph_style(),
        filename,
        l1,
        c1,
        reset_style());
    fprintf(stderr, "%serror:%s ", error_style(), reset_style());
    vfprintf(stderr, fmt, ap);
}

static void
print_flc(
    const char*     filename,
    const char*     source,
    struct str_view loc,
    const char*     fmt,
    ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vflc(filename, source, loc, fmt, ap);
    va_end(ap);
}

static void
print_excerpt(const char* filename, const char* source, struct str_view loc)
{
    int             i;
    int             l1, c1, l2, c2;
    int             indent, max_indent;
    int             gutter_indent;
    int             line;
    struct str_view block;

    /* Calculate line column as well as beginning of block. The goal is to make
     * "block" point to the first character in the line that contains the
     * location. */
    l1 = 1, c1 = 1, block.off = 0;
    for (i = 0; i != loc.off; i++)
    {
        c1++;
        if (source[i] == '\n')
            l1++, c1 = 1, block.off = i + 1;
    }

    /* Calculate line/column of where the location ends */
    l2 = l1, c2 = c1;
    for (i = 0; i != loc.len; i++)
    {
        c2++;
        if (source[loc.off + i] == '\n')
            l2++, c2 = 1;
    }

    /* Find the end of the line for block */
    block.len = loc.off - block.off + loc.len;
    for (; source[loc.off + i]; block.len++, i++)
        if (source[loc.off + i] == '\n')
            break;

    /* We also keep track of the minimum indentation. This is used to unindent
     * the block of code as much as possible when printing out the excerpt. */
    max_indent = 10000;
    for (i = 0; i != block.len;)
    {
        indent = 0;
        for (; i != block.len; ++i, ++indent)
        {
            if (source[block.off + i] != ' ' && source[block.off + i] != '\t')
                break;
        }

        if (max_indent > indent)
            max_indent = indent;

        while (i != block.len)
            if (source[block.off + i++] == '\n')
                break;
    }

    /* Unindent columns */
    c1 -= max_indent;
    c2 -= max_indent;

    /* Find width of the largest line number. This sets the indentation of the
     * gutter */
    gutter_indent = snprintf(NULL, 0, "%d", l2);
    gutter_indent += 2; /* Padding on either side of the line number */

    /* Print line number, gutter, and block of code */
    line = l1;
    for (i = 0; i != block.len;)
    {
        fprintf(stderr, "%*d | ", gutter_indent - 1, line);

        if (i >= loc.off - block.off && i <= loc.off - block.off + loc.len)
            fprintf(stderr, "%s", underline_style());

        indent = 0;
        while (i != block.len)
        {
            if (i == loc.off - block.off)
                fprintf(stderr, "%s", underline_style());
            if (i == loc.off - block.off + loc.len)
                fprintf(stderr, "%s", reset_style());

            if (indent++ >= max_indent)
                putc(source[block.off + i], stderr);

            if (source[block.off + i++] == '\n')
            {
                if (i >= loc.off - block.off
                    && i <= loc.off - block.off + loc.len)
                    fprintf(stderr, "%s", reset_style());
                break;
            }
        }
        line++;
    }
    fprintf(stderr, "%s\n", reset_style());

    /* print underline */
    if (c2 > c1)
    {
        fprintf(stderr, "%*s|%*s", gutter_indent, "", c1, "");
        fprintf(stderr, "%s", underline_style());
        putc('^', stderr);
        for (i = c1 + 1; i < c2; ++i)
            putc('~', stderr);
        fprintf(stderr, "%s", reset_style());
    }
    else
    {
        int col, max_col;

        fprintf(stderr, "%*s| ", gutter_indent, "");
        fprintf(stderr, "%s", underline_style());
        for (i = 1; i < c2; ++i)
            putc('~', stderr);
        for (; i < c1; ++i)
            putc(' ', stderr);
        putc('^', stderr);

        /* Have to find length of the longest line */
        col = 1, max_col = 1;
        for (i = 0; i != block.len; ++i)
        {
            if (max_col < col)
                max_col = col;
            col++;
            if (source[block.off + i] == '\n')
                col = 1;
        }
        max_col -= max_indent;

        for (i = c1 + 1; i < max_col; ++i)
            putc('~', stderr);
        fprintf(stderr, "%s", reset_style());
    }

    putc('\n', stderr);
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
    print_error("%.*s", (int)size, msg);
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
    {
        if (!silence_open_error)
            print_last_win32_error();
        goto open_failed;
    }

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
    {
        print_last_win32_error();
        goto create_file_mapping_failed;
    }

    mf->address = MapViewOfFile(
        mapping,       /* File mapping handle */
        FILE_MAP_READ, /* Read-only view of file */
        0,
        0,  /* High/Low offset of where the mapping should begin in the file */
        0); /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
    {
        print_last_win32_error();
        goto map_view_failed;
    }

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
    {
        print_last_win32_error();
        goto create_file_mapping_failed;
    }

    mf->address = MapViewOfFile(
        mapping,                        /* File mapping handle */
        FILE_MAP_READ | FILE_MAP_WRITE, /* Read + Write */
        0,
        0,  /* High/Low offset of where the mapping should begin in the file */
        0); /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
    {
        print_last_win32_error();
        goto map_view_failed;
    }

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

static int
mfile_map_mem(struct mfile* mf, int size)
{
#if defined(WIN32)
    HANDLE mapping = CreateFileMapping(
        INVALID_HANDLE_VALUE, /* File handle */
        NULL,                 /* Default security attributes */
        PAGE_READWRITE,       /* Read + Write access */
        0,
        size,  /* High/Low size of mapping. Zero means entire file */
        NULL); /* Don't name the mapping */
    if (mapping == NULL)
    {
        print_error(
            "Failed to create file mapping of size %d: {win32error}\n", size);
        goto create_file_mapping_failed;
    }

    mf->address = MapViewOfFile(
        mapping,        /* File mapping handle */
        FILE_MAP_WRITE, /* Read + Write */
        0,
        0, /* High/Low offset of where the mapping should begin in the file */
        size); /* Length of mapping. Zero means entire file */
    if (mf->address == NULL)
    {
        print_error(
            "Failed to map memory of size {emph:%d}: {win32error}\n", size);
        goto map_view_failed;
    }

    CloseHandle(mapping);
    mf->size = size;

    return 0;

map_view_failed:
    CloseHandle(mapping);
create_file_mapping_failed:
    return -1;
#else
    mf->address = mmap(
        NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mf->address == MAP_FAILED)
        return print_error(
            "Failed to mmap() {emph:%d} bytes: %s\n", size, strerror(errno));

    mf->size = size;
    return 0;
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

static int
mfile_map_stdin(struct mfile* mf)
{
    char         b;
    int          len;
    struct mfile new_mf;
    mf->size = 0;

    len = 0;
    while ((b = getc(stdin)) != EOF)
    {
        if (len >= mf->size)
        {
            if (mfile_map_mem(&new_mf, mf->size ? mf->size * 2 : 1024 * 1024)
                != 0)
            {
                return -1;
            }
            memcpy(new_mf.address, mf->address, mf->size);
            if (mf->size)
                mfile_unmap(mf);
            *mf = new_mf;
        }
        ((char*)mf->address)[len++] = b;
    }

    if (len == 0)
        return print_error("Input is empty\n");

    if (mfile_map_mem(&new_mf, len) != 0)
        return -1;
    memcpy(mf->address, new_mf.address, len);
    mfile_unmap(mf);
    *mf = new_mf;

    return 0;
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

/* ----------------------------------------------------------------------------
 * Settings & Command Line
 * ------------------------------------------------------------------------- */

struct cfg
{
    const char* input_fname;
    const char* output_fname;
};

static int
parse_cmdline(int argc, char** argv, struct cfg* cfg)
{
    int i;
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            fprintf(
                stderr,
                "Usage: %s -i <specification filename> -o <output filename>\n",
                argv[0]);
            return 1;
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 >= argc)
                return print_error("Missing input filename to option -i\n");

            cfg->input_fname = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 >= argc)
                return print_error("Missing output filename to option -o\n");

            cfg->output_fname = argv[++i];
        }
    }

    return 0;
}

/* ----------------------------------------------------------------------------
 * Parser
 * ------------------------------------------------------------------------- */

enum token
{
    TOK_ERROR = -1,
    TOK_END = 0,
    TOK_COLON = ':',
    TOK_LBRACE = '{',
    TOK_RBRACE = '}',
    TOK_LT = '<',
    TOK_GT = '>',
    TOK_LBRACKET = '[',
    TOK_RBRACKET = ']',
    TOK_COMMA = ',',
    TOK_OR = '|',
    TOK_SECTION = 256,
    TOK_OPTION,
    TOK_TASK,
    TOK_HELP,
    TOK_SHORT,
    TOK_ARGS,
    TOK_FUNC,
    TOK_RUNAFTER,
    TOK_REQUIRES,
    TOK_ELLIPSIS,
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_CHAR,
};

struct parser
{
    union
    {
        struct str_view str;
        int             integer;
        char            chr;
    } value;
    const char* filename;
    const char* data;
    int         tail;
    int         head;
    int         end;
    enum token  token;
    unsigned    is_peek : 1;
};

static void
parser_init(struct parser* p, struct mfile* mf, const char* filename)
{
    p->filename = filename;
    p->data = (char*)mf->address;
    p->head = 0;
    p->tail = 0;
    p->end = mf->size;
    p->token = TOK_ERROR;
    p->is_peek = 0;
}

static int
print_loc_error(struct parser* p, const char* fmt, ...)
{
    va_list         ap;
    struct str_view loc = {p->tail, p->head - p->tail};

    va_start(ap, fmt);
    log_vflc(p->filename, p->data, loc, fmt, ap);
    va_end(ap);
    print_excerpt(p->filename, p->data, loc);

    return -1;
}

enum token
peek_next(struct parser* p)
{
    if (p->is_peek)
        return p->token;
    p->is_peek = 1;

    p->tail = p->head;
    while (p->head != p->end)
    {
        /* Skip comments */
        if (p->data[p->head] == '/' && p->data[p->head + 1] == '*')
        {
            for (p->head += 2; p->head != p->end; p->head++)
                if (p->data[p->head] == '*' && p->data[p->head + 1] == '/')
                {
                    p->head += 2;
                    break;
                }
            p->tail = p->head;
            continue;
        }
        if (p->data[p->head] == '/' && p->data[p->head + 1] == '/')
        {
            for (p->head += 2; p->head != p->end; p->head++)
                if (p->data[p->head] == '\n')
                {
                    p->head++;
                    break;
                }
            p->tail = p->head;
            continue;
        }
#define SCAN_HELP_STRING(string, tok_name)                                     \
    if (memcmp(p->data + p->head, string, sizeof(string) - 1) == 0)            \
    {                                                                          \
        p->value.str.off = p->head;                                            \
        p->head += sizeof(string) - 1;                                         \
        p->value.str.len = p->head - p->value.str.off;                         \
        return p->token = tok_name;                                            \
    }
        SCAN_HELP_STRING("en_US", TOK_HELP)
#undef SCAN_HELP_STRING
#define SCAN_STRING(string, tok_name)                                          \
    if (memcmp(p->data + p->head, string, sizeof(string) - 1) == 0)            \
    {                                                                          \
        p->head += sizeof(string) - 1;                                         \
        return p->token = tok_name;                                            \
    }
        SCAN_STRING("section", TOK_SECTION)
        SCAN_STRING("option", TOK_OPTION)
        SCAN_STRING("task", TOK_TASK)
        SCAN_STRING("short", TOK_SHORT)
        SCAN_STRING("args", TOK_ARGS)
        SCAN_STRING("func", TOK_FUNC)
        SCAN_STRING("runafter", TOK_RUNAFTER)
        SCAN_STRING("requires", TOK_REQUIRES)
        SCAN_STRING("...", TOK_ELLIPSIS)
#undef SCAN_STRING
#define SCAN_CHAR(char)                                                        \
    if (p->data[p->head] == char)                                              \
        return p->token = p->data[p->head++];
        SCAN_CHAR(':')
        SCAN_CHAR('{')
        SCAN_CHAR('}')
        SCAN_CHAR('<')
        SCAN_CHAR('>')
        SCAN_CHAR('[')
        SCAN_CHAR(']')
        SCAN_CHAR(',')
        SCAN_CHAR('|')
#undef SCAN_CHAR
        /* String literal ".*?" (spans over newlines)*/
        if (p->data[p->head] == '"')
        {
            p->value.str.off = ++p->head;
            for (; p->head != p->end; ++p->head)
                if (p->data[p->head] == '"' && p->data[p->head - 1] != '\\')
                    break;
            if (p->head == p->end)
                return p->token = print_loc_error(
                           p, "Missing closing quote on string\n");
            p->value.str.len = p->head++ - p->value.str.off;
            return p->token = TOK_STRING;
        }
        /* Character literal '.' */
        if (p->data[p->head] == '\'')
        {
            ++p->head;
            if (p->head == p->end || p->data[p->head] == '\'')
                return p->token = print_loc_error(
                           p, "Missing character in character literal\n");
            p->value.chr = p->data[p->head];
            ++p->head;
            if (p->head == p->end || p->data[p->head] != '\'')
                return p->token = print_loc_error(
                           p, "Missing closing quote on character literal\n");
            ++p->head;
            return p->token = TOK_CHAR;
        }
        /* Identifier [a-zA-Z_-][a-zA-Z0-9_-]* */
        if (isalpha(p->data[p->head]) || p->data[p->head] == '_'
            || p->data[p->head] == '-')
        {
            p->value.str.off = p->head++;
            while (p->head != p->end
                   && (isalnum(p->data[p->head]) || p->data[p->head] == '_'
                       || p->data[p->head] == '-'))
            {
                p->head++;
            }
            p->value.str.len = p->head - p->value.str.off;
            return p->token = TOK_IDENTIFIER;
        }

        p->tail = ++p->head;
    }

    return p->token = TOK_END;
}

static enum token
consume(struct parser* p)
{
    p->is_peek = 0;
    return p->token;
}

static enum token
scan_next(struct parser* p)
{
    peek_next(p);
    return consume(p);
}

struct ll
{
    struct ll* next;
};

static void
ll_append(struct ll** head, struct ll* node)
{
    while (*head)
        head = &(*head)->next;
    *head = node;
}

static void
ll_prepend(struct ll** head, struct ll* node)
{
    node->next = *head;
    *head = node;
}

struct strlist
{
    struct strlist* next;
    struct str_view string;
};

struct help_lang
{
    struct help_lang* next;
    struct strlist*   strings;
    struct str_view   lang;
};

struct option
{
    struct option*    next;
    struct help_lang* help;
    struct strlist*   runafter;
    struct strlist*
        requires;
    struct str_view name;
    struct str_view func;
    char            short_name;
};

struct task
{
    struct task*    next;
    struct strlist* runafter;
    struct str_view name;
    struct str_view func;
};

struct section
{
    struct section*   next;
    struct option*    options;
    struct task*      tasks;
    struct help_lang* help;
    struct strlist*   runafter;
    struct str_view   name;
};

struct root
{
    struct section* sections;
};

static struct help_lang*
new_help_lang(struct str_view lang)
{
    struct help_lang* hl = malloc(sizeof *hl);
    hl->next = NULL;
    hl->strings = NULL;
    hl->lang = lang;
    return hl;
}

static struct option*
new_option(struct str_view name)
{
    struct option* o = malloc(sizeof *o);
    o->next = NULL;
    o->help = NULL;
    o->runafter = NULL;
    o->
        requires
    = NULL;
    o->name = name;
    o->func = empty_str_view();
    o->short_name = '\0';
    return o;
}

static struct task*
new_task(struct str_view name)
{
    struct task* t = malloc(sizeof *t);
    t->next = NULL;
    t->runafter = NULL;
    t->name = name;
    t->func = empty_str_view();
    return t;
}

static struct section*
new_section(struct str_view name)
{
    struct section* s = malloc(sizeof *s);
    s->next = NULL;
    s->options = NULL;
    s->tasks = NULL;
    s->help = NULL;
    s->runafter = NULL;
    s->name = name;
    return s;
}

static void
strlist_add(struct strlist** head, struct str_view string)
{
    struct strlist* sl = malloc(sizeof *sl);
    sl->string = string;
    ll_prepend((struct ll**)head, (struct ll*)sl);
}

static int
parse_runafter(struct parser* p, struct strlist** runafter)
{
    if (peek_next(p) == '{')
    {
        consume(p);
        if (scan_next(p) != TOK_IDENTIFIER)
            return print_loc_error(p, "Expected identifier after 'runafter'\n");
        while (peek_next(p) == ',')
        {
            consume(p);
            if (scan_next(p) != TOK_IDENTIFIER)
                return print_loc_error(p, "Expected identifier after ','\n");
            strlist_add(runafter, p->value.str);
        }
        if (scan_next(p) != '}')
            return print_loc_error(
                p, "Missing closing '}' for 'runafter' block\n");

        return 0;
    }

    if (scan_next(p) != TOK_IDENTIFIER)
        return print_loc_error(p, "Expected identifier after 'runafter'\n");
    strlist_add(runafter, p->value.str);

    return 0;
}

static int
parse_requires(struct parser* p, struct strlist** requires)
{
    if (peek_next(p) == '{')
    {
        consume(p);
        if (scan_next(p) != TOK_IDENTIFIER)
            return print_loc_error(p, "Expected identifier after 'requires'\n");
        while (peek_next(p) == ',' || peek_next(p) == '|')
        {
            consume(p);
            if (scan_next(p) != TOK_IDENTIFIER)
                return print_loc_error(
                    p, "Expected identifier after ',' or '|'\n");
            strlist_add(requires, p->value.str);
        }
        if (scan_next(p) != '}')
            return print_loc_error(
                p, "Missing closing '}' for 'requires' block\n");

        return 0;
    }

    if (scan_next(p) != TOK_IDENTIFIER)
        return print_loc_error(p, "Expected identifier after 'requires'\n");
    strlist_add(requires, p->value.str);

    return 0;
}

static int
parse_option(struct parser* p, struct option* option)
{
    while (1)
    {
        switch (peek_next(p))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;

            case TOK_HELP: {
                struct help_lang* help_lang = option->help;
                for (; help_lang; help_lang = help_lang->next)
                    if (str_equal(help_lang->lang, p->value.str, p->data))
                        return print_loc_error(
                            p, "Duplicate 'help' entry for language\n");
                help_lang = new_help_lang(p->value.str);
                help_lang->next = option->help;
                option->help = help_lang;
                consume(p);

                if (scan_next(p) != ':')
                    return print_loc_error(
                        p, "Expected ':' after 'language'\n");
                if (peek_next(p) != TOK_STRING)
                    return print_loc_error(
                        p, "Expected help string after 'language'\n");
                while (peek_next(p) == TOK_STRING)
                {
                    struct strlist* strlist = malloc(sizeof *strlist);
                    strlist->next = NULL;
                    strlist->string = p->value.str;
                    ll_append(
                        (struct ll**)&help_lang->strings, (struct ll*)strlist);
                    consume(p);
                }
                break;
            }

            case TOK_SHORT: {
                if (option->short_name)
                    return print_loc_error(
                        p, "Duplicate 'short' in option block\n");

                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(p, "Expected ':' after 'short'\n");
                if (scan_next(p) != TOK_CHAR)
                    return print_loc_error(
                        p, "Expected character after 'short'\n");
                option->short_name = p->value.chr;

                break;
            }

            case TOK_ARGS: {
                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(p, "Expected ':' after 'args'\n");
                while (peek_next(p) == '[' || peek_next(p) == ']'
                       || peek_next(p) == '<' || peek_next(p) == '>'
                       || peek_next(p) == '|' || peek_next(p) == TOK_IDENTIFIER
                       || peek_next(p) == TOK_ELLIPSIS)
                    consume(p);
                break;
            }

            case TOK_FUNC: {
                if (option->func.len)
                    return print_loc_error(
                        p, "Duplicate 'func' in option block\n");

                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(p, "Expected ':' after 'func'\n");
                if (scan_next(p) != TOK_IDENTIFIER)
                    return print_loc_error(
                        p, "Expected identifier after 'func'\n");
                option->func = p->value.str;

                break;
            }

            case TOK_RUNAFTER: {
                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(
                        p, "Expected ':' after 'runafter'\n");
                if (parse_runafter(p, &option->runafter) != 0)
                    return -1;
                break;
            }

            case TOK_REQUIRES: {
                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(
                        p, "Expected ':' after 'requires'\n");
                if (parse_requires(p, &option->requires) != 0)
                    return -1;
                break;
            }

            case '}': return 0;

            default:
                return print_loc_error(p, "Unexpected token in option block\n");
        }
    }
}

static int
parse_task(struct parser* p, struct task* task)
{
    while (1)
    {
        switch (peek_next(p))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;

            case TOK_FUNC: {
                if (task->func.len)
                    return print_loc_error(
                        p, "Duplicate 'func' in task block\n");

                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(p, "Expected ':' after 'func'\n");
                if (scan_next(p) != TOK_IDENTIFIER)
                    return print_loc_error(
                        p, "Expected identifier after 'func'\n");

                break;
            }

            case TOK_RUNAFTER: {
                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(
                        p, "Expected ':' after 'runafter'\n");
                if (parse_runafter(p, &task->runafter) != 0)
                    return -1;
                break;
            }

            case '}': return 0;

            default:
                return print_loc_error(p, "Unexpected token in task block\n");
        }
    }
}

static int
parse_section(struct parser* p, struct section* section)
{
    while (1)
    {
        switch (peek_next(p))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;

            case TOK_OPTION: {
                struct option* option;

                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(p, "Expected ':' after 'option'\n");
                if (scan_next(p) != TOK_IDENTIFIER)
                    return print_loc_error(
                        p, "Expected option name after 'option'\n");

                option = new_option(p->value.str);
                if (scan_next(p) != '{')
                    return print_loc_error(
                        p, "Expected opening '{' for option block\n");
                if (parse_option(p, option) != 0)
                    return -1;
                if (scan_next(p) != '}')
                    return print_loc_error(
                        p, "Missing closing '}' for previous option block\n");

                option->next = section->options;
                section->options = option;
                break;
            }

            case TOK_TASK: {
                struct task* task;

                consume(p);
                if (scan_next(p) != ':')
                    return print_loc_error(p, "Expected ':' after 'task'\n");
                if (scan_next(p) != TOK_IDENTIFIER)
                    return print_loc_error(
                        p, "Expected identifier after 'task'\n");

                task = new_task(p->value.str);
                if (scan_next(p) != '{')
                    return print_loc_error(
                        p, "Expected opening '{' for option block\n");
                if (parse_task(p, task) != 0)
                    return -1;
                if (scan_next(p) != '}')
                    return print_loc_error(
                        p, "Missing closing '}' for previous option block\n");

                task->next = section->tasks;
                section->tasks = task;
                break;
            }

            case TOK_HELP: {
                struct help_lang* help_lang = section->help;
                for (; help_lang; help_lang = help_lang->next)
                    if (str_equal(help_lang->lang, p->value.str, p->data))
                        return print_loc_error(
                            p, "Duplicate 'help' entry for language\n");
                help_lang = new_help_lang(p->value.str);
                help_lang->next = section->help;
                section->help = help_lang;
                consume(p);

                if (scan_next(p) != ':')
                    return print_loc_error(
                        p, "Expected ':' after 'language'\n");
                if (peek_next(p) != TOK_STRING)
                    return print_loc_error(
                        p, "Expected help string after 'language'\n");
                while (peek_next(p) == TOK_STRING)
                {
                    struct strlist* strlist = malloc(sizeof *strlist);
                    strlist->next = NULL;
                    strlist->string = p->value.str;
                    ll_append(
                        (struct ll**)&help_lang->strings, (struct ll*)strlist);
                    consume(p);
                }
                break;
            }

            case '}': return 0;

            default:
                return print_loc_error(
                    p, "Unexpected token in section block\n");
        }
    }
}

static int
parse(struct parser* p, struct root* root, const struct cfg* cfg)
{
    while (1)
    {
        switch (scan_next(p))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;

            case TOK_SECTION: {
                struct section* section;
                if (scan_next(p) == ':')
                {
                    if (scan_next(p) != TOK_IDENTIFIER)
                        return print_loc_error(
                            p, "Expected section name after ':'\n");
                    section = new_section(p->value.str);
                }
                else
                    section = new_section(empty_str_view());

                if (scan_next(p) != '{')
                    return print_loc_error(
                        p, "Expected opening '{' for section block\n");
                if (parse_section(p, section) != 0)
                    return -1;
                if (scan_next(p) != '}')
                    return print_loc_error(
                        p, "Missing closing '}' for previous section block\n");

                section->next = root->sections;
                root->sections = section;
                break;
            }

            default: return print_loc_error(p, "Unexpected token\n");
        }
    }
}

/* ----------------------------------------------------------------------------
 * Dependency solver
 * ------------------------------------------------------------------------- */

struct table_entry
{
    struct str_view long_opt;
    struct str_view short_opt;
    int             priority;
};

static void
gen_table(struct mstream* ms, const struct root* root, const char* data)
{
#if defined(_WIN32)
#define NL "\r\n"
#else
#define NL "\n"
#endif
    struct section* section;

    mstream_fmt(ms, "static const int commands[] = {" NL);
    for (section = root->sections; section; section = section->next)
    {
        struct option* option;
        for (option = section->options; option; option = option->next)
        {
            struct help_lang* help_lang = option->help;
            for (; help_lang; help_lang = help_lang->next)
            {
                struct strlist* strs;
                if (!cstr_equal("en_US", help_lang->lang, data))
                    continue;

                mstream_cstr(ms, "    .en_US = \"");
                strs = help_lang->strings;
                for (; strs; strs = strs->next)
                    mstream_str(ms, strs->string, data);
                mstream_cstr(ms, "\"," NL);
            }
        }
    }
    mstream_fmt(ms, "};" NL);
}

static int
write_if_different(const struct mstream* ms, const char* filename)
{
    struct mfile mf;

    /* Don't write resource if it is identical to the existing one -- causes
     * less rebuilds */
    if (mfile_map_read(&mf, filename, 1) == 0)
    {
        if (mf.size == ms->write_ptr
            && memcmp(mf.address, ms->address, mf.size) == 0)
            return 0;
        mfile_unmap(&mf);
    }

    /* Write out file */
    if (mfile_map_write(&mf, filename, ms->write_ptr) != 0)
        return -1;
    memcpy(mf.address, ms->address, ms->write_ptr);
    mfile_unmap(&mf);

    return 0;
}

int
main(int argc, char** argv)
{
    struct mfile   mf;
    struct mstream ms;
    struct parser  parser;
    struct cfg     cfg = {0};
    struct root    root = {0};

    if (!stream_is_terminal(stderr))
        disable_colors = 1;

    if (parse_cmdline(argc, argv, &cfg) != 0)
        return -1;

    if (cfg.input_fname == NULL)
    {
        if (mfile_map_stdin(&mf) != 0)
            return -1;
    }
    else
    {
        if (mfile_map_read(&mf, cfg.input_fname, 0) != 0)
            return -1;
    }

    parser_init(&parser, &mf, cfg.input_fname);
    if (parse(&parser, &root, &cfg) != 0)
        return -1;

    ms = mstream_init_writeable();
    gen_table(&ms, &root, mf.address);

    if (cfg.output_fname == NULL)
        fwrite(ms.address, ms.write_ptr, 1, stdout);
    else if (write_if_different(&ms, cfg.output_fname) != 0)
        return -1;

    return 0;
}
