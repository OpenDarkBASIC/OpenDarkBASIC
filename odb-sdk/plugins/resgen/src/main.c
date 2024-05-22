#if defined(_WIN32)
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

/* ------------------------------------------------------------------------- */
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
                return print_error("Missing argument to option -i\n");

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
                return print_error("Missing argument to option -o\n");

            cfg->output_file = argv[++i];
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            const char* target_name;
            if (i + 1 >= argc)
                return print_error("Missing argument to option -t\n");

            target_name = argv[++i];
            if (strcmp(target_name, "winres") == 0)
                cfg->target = TARGET_WINRES;
            else if (strcmp(target_name, "elf") == 0)
                cfg->target = TARGET_ELF;
            else
                return print_error("Unknown target '%s'\n", target_name);
        }
    }

    if (cfg->target == TARGET_NONE)
        return print_error(
            "No target was specified. Use -t <winres|elf> to specify the "
            "output format\n");

    if (cfg->input_files == NULL || cfg->input_files_count == 0)
        return print_error("No input files specified. Use -i <files...>\n");

    if (cfg->output_file == NULL)
        return print_error("No output file specified. Use -o <output>\n");

    return 0;
}

/* ----------------------------------------------------------------------------
 * Parser
 * ------------------------------------------------------------------------- */

struct parser
{
    const char* filename;
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
parser_init(struct parser* p, struct mfile* mf, const char* filename)
{
    p->filename = filename;
    p->data = (char*)mf->address;
    p->head = 0;
    p->tail = 0;
    p->len = mf->size;
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
{
    TOK_ERROR = -1,
    TOK_END = 0,
    TOK_LPAREN = '(',
    TOK_RPAREN = ')',
    TOK_COMMA = ',',
    TOK_ASTERISK = '*',
    TOK_ODB_COMMAND = 256,
    TOK_ODB_OVERLOAD,
    TOK_COMMAND_NAME,
    TOK_COMMAND_BRIEF,
    TOK_COMMAND_DESCRIPTION,
    TOK_COMMAND_PARAM,
    TOK_COMMAND_RETURNS,
    TOK_COMMAND_EXAMPLE,
    TOK_COMMAND_SEE_ALSO,
    TOK_ELLIPSIS,
    TOK_IDENTIFIER,
    TOK_STRING,
};

enum token
scan_next_token(struct parser* p)
{
    p->tail = p->head;
    while (p->head != p->len)
    {
        /* Skip comments */
        if (p->data[p->head] == '/' && p->data[p->head + 1] == '*')
        {
            for (p->head += 2; p->head != p->len; p->head++)
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
            for (p->head += 2; p->head != p->len; p->head++)
                if (p->data[p->head] == '\n')
                {
                    p->head++;
                    break;
                }
            p->tail = p->head;
            continue;
        }
        /* String literals. Regex: ".*?" (spans over newlines)*/
        if (p->data[p->head] == '"'
            && (p->head == 0 || p->data[p->head - 1] != '\\'))
        {
            p->value.str.off = ++p->head;
            for (; p->head != p->len; ++p->head)
                if (p->data[p->head] == '"' && p->data[p->head - 1] != '\\')
                    break;
            if (p->head == p->len)
                return print_loc_error(p, "Missing closing quote on string\n");
            p->value.str.len = p->head++ - p->value.str.off;
            return TOK_STRING;
        }
        if (p->data[p->head] == '(')
            return p->data[p->head++];
        if (p->data[p->head] == ')')
            return p->data[p->head++];
        if (p->data[p->head] == ',')
            return p->data[p->head++];
        if (p->data[p->head] == '*')
            return p->data[p->head++];
        if (memcmp(p->data + p->head, "ODB_COMMAND", sizeof("ODB_COMMAND") - 1)
            == 0)
        {
            p->head += sizeof("ODB_COMMAND") - 1;
            while (p->head != p->len && isdigit(p->data[p->head]))
                p->head++;
            return TOK_ODB_COMMAND;
        }
        if (memcmp(
                p->data + p->head, "ODB_OVERLOAD", sizeof("ODB_OVERLOAD") - 1)
            == 0)
        {
            p->head += sizeof("ODB_OVERLOAD") - 1;
            while (p->head != p->len && isdigit(p->data[p->head]))
                p->head++;
            return TOK_ODB_OVERLOAD;
        }
        if (memcmp(p->data + p->head, "NAME", sizeof("NAME") - 1) == 0)
        {
            p->head += sizeof("NAME") - 1;
            return TOK_COMMAND_NAME;
        }
        if (memcmp(p->data + p->head, "BRIEF", sizeof("BRIEF") - 1) == 0)
        {
            p->head += sizeof("BRIEF") - 1;
            return TOK_COMMAND_BRIEF;
        }
        if (memcmp(p->data + p->head, "DESCRIPTION", sizeof("DESCRIPTION") - 1)
            == 0)
        {
            p->head += sizeof("DESCRIPTION") - 1;
            return TOK_COMMAND_DESCRIPTION;
        }
        if (memcmp(p->data + p->head, "PARAMETER", sizeof("PARAMETER") - 1)
            == 0)
        {
            p->head += sizeof("PARAMETER") - 1;
            while (p->head != p->len && isdigit(p->data[p->head]))
                p->head++;
            return TOK_COMMAND_PARAM;
        }
        if (memcmp(p->data + p->head, "RETURNS", sizeof("RETURNS") - 1) == 0)
        {
            p->head += sizeof("RETURNS") - 1;
            return TOK_COMMAND_RETURNS;
        }
        if (memcmp(p->data + p->head, "EXAMPLE", sizeof("EXAMPLE") - 1) == 0)
        {
            p->head += sizeof("EXAMPLE") - 1;
            return TOK_COMMAND_EXAMPLE;
        }
        if (memcmp(p->data + p->head, "SEE_ALSO", sizeof("SEE_ALSO") - 1) == 0)
        {
            p->head += sizeof("SEE_ALSO") - 1;
            return TOK_COMMAND_SEE_ALSO;
        }
        if (memcmp(p->data + p->head, "...", 3) == 0)
        {
            p->head += 3;
            return TOK_ELLIPSIS;
        }
        if (isalpha(p->data[p->head]) || p->data[p->head] == '_')
        {
            p->value.str.off = p->head++;
            while (p->head != p->len
                   && (isalnum(p->data[p->head]) || p->data[p->head] == '_'))
            {
                p->head++;
            }
            p->value.str.len = p->head - p->value.str.off;
            return TOK_IDENTIFIER;
        }

        p->tail = ++p->head;
    }

    return TOK_END;
}

enum param_type
{
    PARAM_NONE = 0,

    PARAM_VOID = '0',
    PARAM_LONG = 'R',    /* 8 bytes -- signed int */
    PARAM_DWORD = 'D',   /* 4 bytes -- unsigned int */
    PARAM_INTEGER = 'L', /* 4 bytes -- signed int */
    PARAM_WORD = 'W',    /* 2 bytes -- unsigned int */
    PARAM_BYTE = 'Y',    /* 1 byte -- unsigned int */
    PARAM_BOOLEAN = 'B', /* 1 byte -- boolean */
    PARAM_FLOAT = 'F',   /* 4 bytes -- float */
    PARAM_DOUBLE = 'O',  /* 8 bytes -- double */
    PARAM_STRING = 'S',  /* 4 bytes -- char* (passed as DWORD on 32-bit) */
    PARAM_ARRAY = 'H',   /* 4 bytes -- Pass array address directly */
    PARAM_LABEL = 'P',   /* 4 bytes -- ? */
    PARAM_DABEL = 'Q',   /* 4 bytes -- ? */
    PARAM_ANY = 'X',     /* 4 bytes -- (think reinterpret_cast) */
    PARAM_USER_DEFINED_VAR_PTR = 'E', /* 4 bytes */
};

struct param
{
    struct param* next;

    enum param_type type;

    unsigned is_ptr : 1;
    unsigned is_const : 1;
    unsigned is_signed : 1;
    unsigned is_unsigned : 1;
    unsigned is_char : 1;
    unsigned is_struct : 1;
};

/* Description can span multiple lines, split into string fragments */
struct param_doc_desc
{
    struct param_doc_desc* next;
    struct str_view        text;
};

struct param_doc
{
    struct param_doc* next;

    struct str_view        name;
    struct param_doc_desc* desc;
};

struct cmd
{
    struct cmd*       next;
    struct param*     params;
    struct param_doc* param_docs;

    const char* source;

    struct str_view name;
    struct str_view symbol;
    struct param    ret;

    unsigned is_overload : 1;
};

struct root
{
    struct cmd* commands;
};

static struct cmd*
new_command(struct root* root)
{
    struct cmd** cmd = &root->commands;
    while (*cmd)
        cmd = &(*cmd)->next;

    *cmd = calloc(1, sizeof **cmd);
    return *cmd;
}

static struct param*
new_param(struct cmd* command)
{
    struct param** param = &command->params;
    while (*param)
        param = &(*param)->next;

    *param = calloc(1, sizeof **param);
    return *param;
}

static struct param_doc*
new_param_doc(struct cmd* cmd)
{
    struct param_doc** doc = &cmd->param_docs;
    while (*doc)
        doc = &(*doc)->next;

    *doc = calloc(1, sizeof **doc);
    return *doc;
}

static struct param_doc_desc*
new_param_doc_desc(struct param_doc* doc)
{
    struct param_doc_desc** desc = &doc->desc;
    while (*desc)
        desc = &(*desc)->next;

    *desc = calloc(1, sizeof **desc);
    return *desc;
}

static enum token
parse_parameter(struct parser* p, struct cmd* command, char is_ret)
{
    enum token    tok;
    struct param* param = NULL;

    while (1)
    {
        switch ((tok = scan_next_token(p)))
        {
            case TOK_ERROR:
            case TOK_END:
            default: return tok;

            /* Do any post processing on the parameter and then return */
            case ',':
            case ')':
                if (param)
                {
                    /* If a pointer was detected, change the type to an array,
                     * unless it was also a "char". Then it is a string */
                    if (param->is_ptr)
                    {
                        if (param->is_char)
                            param->type = PARAM_STRING;
                        else
                            param->type = PARAM_ARRAY;
                    }
                }
                return tok;

            /* Alloc if not yet done */
            case '*':
            case TOK_ELLIPSIS:
            case TOK_IDENTIFIER:
                if (param == NULL)
                    param = is_ret ? &command->ret : new_param(command);
                break;
        }

        switch (tok)
        {
            case '*': param->is_ptr = 1; break;
            case TOK_ELLIPSIS: param->type = PARAM_USER_DEFINED_VAR_PTR; break;
            case TOK_IDENTIFIER:
                /* C qualifiers */
                if (memcmp(p->data + p->value.str.off, "const", 5) == 0)
                    param->is_const = 1;
                else if (memcmp(p->data + p->value.str.off, "signed", 6) == 0)
                    param->is_signed = 1;
                else if (memcmp(p->data + p->value.str.off, "unsigned", 8) == 0)
                    param->is_unsigned = 1;
                else if (memcmp(p->data + p->value.str.off, "struct", 6) == 0)
                    param->is_struct = 1;
                /* C types */
                else if (memcmp(p->data + p->value.str.off, "void", 4) == 0)
                    param->type = PARAM_VOID;
                else if (memcmp(p->data + p->value.str.off, "float", 5) == 0)
                    param->type = PARAM_FLOAT;
                else if (memcmp(p->data + p->value.str.off, "double", 6) == 0)
                    param->type = PARAM_DOUBLE;
                else if (memcmp(p->data + p->value.str.off, "int64_t", 7) == 0)
                    param->type = PARAM_LONG;
                else if (memcmp(p->data + p->value.str.off, "uint64_t", 8) == 0)
                    param->type = PARAM_LONG;
                else if (memcmp(p->data + p->value.str.off, "int", 3) == 0)
                    param->type
                        = param->is_unsigned ? PARAM_DWORD : PARAM_INTEGER;
                else if (memcmp(p->data + p->value.str.off, "int32_t", 7) == 0)
                    param->type
                        = param->is_unsigned ? PARAM_DWORD : PARAM_INTEGER;
                else if (memcmp(p->data + p->value.str.off, "uint32_t", 8) == 0)
                    param->type = PARAM_DWORD;
                else if (memcmp(p->data + p->value.str.off, "int16_t", 7) == 0)
                    param->type = PARAM_WORD;
                else if (memcmp(p->data + p->value.str.off, "uint16_t", 8) == 0)
                    param->type = PARAM_WORD;
                else if (memcmp(p->data + p->value.str.off, "char", 4) == 0)
                {
                    param->type = PARAM_BYTE;
                    param->is_char = 1;
                }
                else if (memcmp(p->data + p->value.str.off, "uint8_t", 7) == 0)
                    param->type = PARAM_BYTE;
                else if (memcmp(p->data + p->value.str.off, "int8_t", 6) == 0)
                    param->type = PARAM_BYTE;
                else if (memcmp(p->data + p->value.str.off, "bool", 4) == 0)
                    param->type = PARAM_BOOLEAN;
                else if (memcmp(p->data + p->value.str.off, "_Bool", 5) == 0)
                    param->type = PARAM_BOOLEAN;
                else
                {
                    if (param->type == PARAM_NONE)
                        return print_loc_error(
                            p,
                            "Unknown type '%.*s' encountered: Don't know how "
                            "to map to DB type. This is an issue with "
                            "odb-resgen. Please report a bug!\n",
                            p->value.str.len,
                            p->data + p->value.str.off);
                }
                break;

            default: break;
        }
    }
}

static enum token
parse_c_parameters(struct parser* p, struct cmd* command)
{
    enum token tok;
    while (1)
    {
        switch ((tok = parse_parameter(p, command, 0)))
        {
            case ',': break;
            default: return tok;
        }
    }
}

static enum token
parse_doc_parameters(struct parser* p, struct cmd* cmd)
{
    enum token tok;
    while (1)
    {
        switch ((tok = scan_next_token(p)))
        {
            case ',': break;
            default: return tok;

            case TOK_COMMAND_BRIEF:
            case TOK_COMMAND_DESCRIPTION:
            case TOK_COMMAND_RETURNS:
            case TOK_COMMAND_EXAMPLE:
            case TOK_COMMAND_SEE_ALSO:
                if (scan_next_token(p) != '(')
                    return print_loc_error(
                        p, "Expected argument to BRIEF() macro\n");
                while (1)
                {
                    switch ((tok = scan_next_token(p)))
                    {
                        case ',':
                        case TOK_STRING: continue;
                        case ')': break;

                        default: return tok;
                    }
                    break;
                }
                break;

            case TOK_COMMAND_PARAM: {
                struct param_doc* doc;

                if (scan_next_token(p) != '(')
                    return print_loc_error(
                        p, "Expected argument to PARAMETER() macro\n");
                if (scan_next_token(p) != TOK_STRING)
                    return print_loc_error(
                        p,
                        "Expected parameter name as argument to PARAMETER() "
                        "macro\n");

                doc = new_param_doc(cmd);
                doc->name = p->value.str;

                if (scan_next_token(p) != ',')
                    return print_loc_error(
                        p,
                        "Expected a string containing a description of this "
                        "parameter\n");

                while ((tok = scan_next_token(p)) == TOK_STRING)
                {
                    struct param_doc_desc* desc = new_param_doc_desc(doc);
                    desc->text = p->value.str;
                }

                if (tok != ')')
                    return print_loc_error(p, "Missing closing ')'\n");
                break;
            }
        }
    }
}

static enum token
parse_command(struct parser* p, struct cmd* command, char is_overload)
{
    command->source = p->data;

    if (scan_next_token(p) != '(')
        return print_loc_error(p, "Expected argument list\n");

    /* C function return value */
    switch (parse_parameter(p, command, 1))
    {
        case ',': break;
        case ')':
            return print_loc_error(
                p,
                "Expected C function name as second argument, but "
                "ODB_COMMAND() macro was only given 1 argument\n");
        default:
            return print_loc_error(
                p,
                "Expected function return type as first argument to "
                "ODB_COMMAND()\n");
    }

    if (scan_next_token(p) != TOK_IDENTIFIER)
        return print_loc_error(
            p,
            "Expected function name as second argument to ODB_COMMAND() "
            "macro\n");
    command->symbol = p->value.str;

    /* Parse parameter types of C function */
    if (parse_c_parameters(p, command) != TOK_COMMAND_NAME)
        return print_loc_error(
            p,
            "Expected NAME(\"<command name>\") as next argument to "
            "ODB_COMMAND()\n");

    /* Parse command name */
    if (scan_next_token(p) != '(')
        return print_loc_error(p, "Expected argument to NAME() macro\n");

    if (scan_next_token(p) != TOK_STRING)
        return print_loc_error(
            p,
            "Expected command name string as argument to "
            "NAME() macro\n");
    command->name = p->value.str;

    if (scan_next_token(p) != ')')
        return print_loc_error(p, "Missing closing ')'\n");

    if (is_overload)
    {
        if (scan_next_token(p) != ')')
            return print_loc_error(
                p,
                "Too many arguments passed to ODB_OVERLOAD. Note that "
                "ODB_OVERLOAD "
                "does not accept any of the documentation arguments that "
                "ODB_COMMAND accepts. The documentation of overloaded "
                "functions is "
                "shared between all overloads.\n");

        command->is_overload = 1;
        return ')';
    }
    return parse_doc_parameters(p, command);
}

static struct cmd*
find_command_by_name(
    const struct parser* p, const struct root* root, struct str_view name)
{
    struct cmd* cmd;
    for (cmd = root->commands; cmd; cmd = cmd->next)
        if (name.len == cmd->name.len
            && memcmp(p->data + name.off, p->data + cmd->name.off, name.len)
                   == 0)
            return cmd;
    return NULL;
}

static int
parse(struct parser* p, struct root* root, const struct cfg* cfg)
{
    enum token tok;
    while (1)
    {
        switch ((tok = scan_next_token(p)))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;

            case TOK_ODB_COMMAND:
                if (parse_command(p, new_command(root), 0) != ')')
                    return print_loc_error(
                        p, "Unexpected token encountered.\n");
                break;

            case TOK_ODB_OVERLOAD: {
                struct cmd*       base;
                struct cmd*       ol;
                struct param*     ol_param;
                struct param_doc* base_doc;

                ol = new_command(root);
                if (parse_command(p, ol, 1) != ')')
                    return print_loc_error(
                        p, "Unexpected token encountered.\n");

                base = find_command_by_name(p, root, ol->name);
                if (base == NULL)
                    return print_loc_error(
                        p,
                        "Command overload not found for \"%.*s\"\n",
                        ol->name.len,
                        p->data + ol->name.off);

                /* Copy parameter names from base command, since they are shared
                 */
                base_doc = base->param_docs;
                ol_param = ol->params;
                while (ol_param && base_doc)
                {
                    struct param_doc* ol_doc = new_param_doc(ol);
                    ol_doc->name = base_doc->name;

                    ol_param = ol_param->next;
                    base_doc = base_doc->next;
                }
            }
            break;

            default: break;
        }
    }
}

static void
gen_command_string(struct mstream* ms, const struct cmd* cmd)
{
    struct param*     param;
    struct param_doc* param_doc;
    mstream_fmt(ms, "%S", cmd->name, cmd->source);
    mstream_putc(ms, '%');
    mstream_putc(ms, cmd->ret.type);

    mstream_putc(ms, '(');
    param = cmd->params;
    while (param)
    {
        mstream_putc(ms, param->type);
        param = param->next;
    }
    mstream_putc(ms, ')');

    mstream_putc(ms, '%');
    mstream_fmt(ms, "%S", cmd->symbol, cmd->source);
    mstream_putc(ms, '%');

    param = cmd->params;
    param_doc = cmd->param_docs;
    while (param && param_doc)
    {
        if (param != cmd->params)
            mstream_cstr(ms, ", ");

        mstream_fmt(ms, "%S", param_doc->name, cmd->source);
        param = param->next;
        param_doc = param_doc->next;
    }
}

static void
gen_winres_resource(struct mstream* ms, const struct root* root)
{
    int               stringID = 1;
    const struct cmd* cmd = root->commands;

    mstream_cstr(ms, "STRINGTABLE\n");
    mstream_cstr(ms, "BEGIN\n");

    while (cmd)
    {
        mstream_fmt(ms, "    %d", stringID++);
        mstream_cstr(ms, " \"");
        gen_command_string(ms, cmd);
        mstream_cstr(ms, "\"\n");
        cmd = cmd->next;
    }

    mstream_cstr(ms, "END\n");
}

static void
gen_elf_resource(struct mstream* ms, const struct root* root)
{
    const struct cmd* cmd = root->commands;
    while (cmd)
    {
        if (cmd != root->commands)
            mstream_putc(ms, '\n');

        gen_command_string(ms, cmd);
        cmd = cmd->next;
    }
}

static int
write_resource(const struct mstream* ms, const char* filename)
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
    struct parser  parser;
    struct cfg     cfg = {0};
    struct root    root = {0};
    struct mstream ms = mstream_init_writeable();

    if (!stream_is_terminal(stderr))
        disable_colors = 1;

    if (parse_cmdline(argc, argv, &cfg) != 0)
        return -1;

    for (int i = 0; i != cfg.input_files_count; ++i)
    {
        struct mfile mf;

        /* File names can be empty if CMake uses generator expressions for file
         * names */
        if (!*cfg.input_files[i])
            continue;

        if (mfile_map_read(&mf, cfg.input_files[i], 0) != 0)
            return -1;

        parser_init(&parser, &mf, cfg.input_files[i]);
        if (parse(&parser, &root, &cfg) != 0)
            return -1;
    }

    switch (cfg.target)
    {
        case TARGET_NONE:
            return print_error("No target specified. Use -t <winres|elf>\n");

        case TARGET_WINRES: gen_winres_resource(&ms, &root); break;
        case TARGET_ELF: gen_elf_resource(&ms, &root); break;
    }

    if (write_resource(&ms, cfg.output_file) != 0)
        return -1;

    return 0;
}
