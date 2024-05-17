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
    TOK_LPAREN = '(',
    TOK_RPAREN = ')',
    TOK_COMMA = ',',
    TOK_ASTERISK = '*',
    TOK_ODB_COMMAND = 256,
    TOK_IDENTIFIER,
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

        p->head++;
    }

    return TOK_END;
}

enum arg_type
{
    ARG_NONE = 0,

    ARG_VOID = '0',
    ARG_LONG = 'R',    /* 8 bytes -- signed int */
    ARG_DWORD = 'D',   /* 4 bytes -- unsigned int */
    ARG_INTEGER = 'L', /* 4 bytes -- signed int */
    ARG_WORD = 'W',    /* 2 bytes -- unsigned int */
    ARG_BYTE = 'Y',    /* 1 byte -- unsigned int */
    ARG_BOOLEAN = 'B', /* 1 byte -- boolean */
    ARG_FLOAT = 'F',   /* 4 bytes -- float */
    ARG_DOUBLE = 'O',  /* 8 bytes -- double */
    ARG_STRING = 'S',  /* 4 bytes -- char* (passed as DWORD on 32-bit) */
    ARG_ARRAY = 'H',   /* 4 bytes -- Pass array address directly */
    ARG_LABEL = 'P',   /* 4 bytes -- ? */
    ARG_DABEL = 'Q',   /* 4 bytes -- ? */
    ARG_ANY = 'X',     /* 4 bytes -- (think reinterpret_cast) */
    ARG_USER_DEFINED_VAR_PTR = 'E', /* 4 bytes */
};

struct arg
{
    struct arg* next;

    struct str_view name;
    enum arg_type   type;

    unsigned is_ptr : 1;
    unsigned is_const : 1;
};

struct command
{
    struct command* next;
    struct arg*     args;

    struct str_view name;
    struct str_view help;
    struct str_view symbol;
    struct arg      ret;
};

struct root
{
    struct command* commands;
};

static struct command*
new_command(struct root* root)
{
    struct command** cmd = &root->commands;
    while (*cmd)
        cmd = &(*cmd)->next;

    *cmd = calloc(1, sizeof **cmd);
    return *cmd;
}

static struct arg*
new_argument(struct command* command)
{
    struct arg** arg = &command->args;
    while (*arg)
        arg = &(*arg)->next;

    *arg = calloc(1, sizeof **arg);
    return *arg;
}

static enum token
parse_argument(struct parser* p, struct command* command, char is_ret)
{
    enum token  tok;
    struct arg* arg = NULL;

    while (1)
    {
        switch ((tok = scan_next_token(p)))
        {
            case TOK_ERROR:
            case TOK_END:
            default: return TOK_ERROR;

            case ',':
            case ')':
                if (arg)
                {
                    if (arg->is_ptr)
                        arg->type = ARG_ARRAY;
                }
                return tok;

            case '*':
            case TOK_IDENTIFIER:
                if (arg == NULL)
                    arg = is_ret ? &command->ret : new_argument(command);
                break;
        }

        switch (tok)
        {
            case '*': arg->is_ptr = 1; break;
            case TOK_IDENTIFIER:
                if (memcmp(p->data + p->value.str.off, "const", 4) == 0)
                    arg->is_const = 1;
                else if (memcmp(p->data + p->value.str.off, "void", 4) == 0)
                    arg->type = ARG_VOID;
                else if (memcmp(p->data + p->value.str.off, "int", 3) == 0)
                    arg->type = ARG_INTEGER;
                else if (memcmp(p->data + p->value.str.off, "char", 3) == 0)
                    arg->type = ARG_INTEGER;
                else
                {
                    arg->name = p->value.str;
                    if (arg->type == ARG_NONE)
                    {
                        fprintf(
                            stderr,
                            "Error: Unknown type '%.*s' encountered: Don't "
                            "know "
                            "how "
                            "to map to DB type. This is an issue with "
                            "odb-resgen. "
                            "Please report a bug!\n",
                            p->value.str.len,
                            p->data + p->value.str.off);
                        return TOK_ERROR;
                    }
                }
                break;

            default: break;
        }
    }
}

static enum token
parse_command(struct parser* p, struct command* command)
{
    enum token tok;

    if (scan_next_token(p) != '(')
        return print_error(p, "Error: Expected argument list\n");

    if (scan_next_token(p) != TOK_STRING)
        return print_error(
            p,
            "Error: Expected command name string as first parameter to "
            "ODB_COMMAND()\n");
    command->name = p->value.str;

    if (scan_next_token(p) != ',')
        return print_error(p, "Error: Expected next argument\n");

    if (scan_next_token(p) != TOK_STRING)
        return print_error(
            p,
            "Error: Expected help file string as second parameter to "
            "ODB_COMMAND()\n");
    command->help = p->value.str;

    if (scan_next_token(p) != ',')
        return print_error(p, "Error: Expected next argument\n");

    switch (parse_argument(p, command, 1))
    {
        case ',': break;
        case ')': return print_error(p, "Error: Expected next argument\n");
        default:
            return print_error(
                p,
                "Error: Expected function return type as third parameter to "
                "ODB_COMMAND()\n");
    }

    if (scan_next_token(p) != TOK_IDENTIFIER)
        return print_error(
            p,
            "Error: Expected function name as fourth parameter to "
            "ODB_COMMAND()\n");
    command->symbol = p->value.str;

    while (1)
    {
        switch ((tok = parse_argument(p, command, 0)))
        {
            case ',': break;
            default: return tok;
        }
    }
}

static int
parse(struct parser* p, struct root* root, const struct cfg* cfg)
{
    while (1)
    {
        switch (scan_next_token(p))
        {
            case TOK_ERROR: return -1;
            case TOK_END: return 0;
            case TOK_ODB_COMMAND:
                if (parse_command(p, new_command(root)) != ')')
                    return -1;
            default: break;
        }
    }
}

static int
gen_winres_resource(struct mstream* ms)
{
    return 0;
}

static int
gen_elf_resource(struct mstream* ms, const struct root* root, const char* data)
{
    const struct command* cmd = root->commands;
    const struct arg*     arg;
    while (cmd)
    {
        mstream_fmt(ms, "%S", cmd->name, data);
        mstream_putc(ms, '%');
        mstream_putc(ms, cmd->ret.type);

        mstream_putc(ms, '(');
        arg = cmd->args;
        while (arg)
        {
            mstream_putc(ms, arg->type);
            arg = arg->next;
        }
        mstream_putc(ms, ')');

        mstream_putc(ms, '%');
        mstream_fmt(ms, "%S", cmd->symbol, data);
        mstream_putc(ms, '%');

        arg = cmd->args;
        while (arg)
        {
            if (arg != cmd->args)
                mstream_cstr(ms, ", ");
            mstream_fmt(ms, "%S", arg->name, data);
            switch (arg->type)
            {
                case ARG_NONE: break;
                case ARG_VOID: break;
                case ARG_LONG: mstream_cstr(ms, " as long"); break;
                case ARG_DWORD: mstream_cstr(ms, " as dword"); break;
                case ARG_INTEGER: mstream_cstr(ms, " as integer"); break;
                case ARG_WORD: mstream_cstr(ms, " as word"); break;
                case ARG_BYTE: mstream_cstr(ms, " as byte"); break;
                case ARG_BOOLEAN: mstream_cstr(ms, " as boolean"); break;
                case ARG_FLOAT: mstream_cstr(ms, " as float"); break;
                case ARG_DOUBLE: mstream_cstr(ms, " as double"); break;
                case ARG_STRING: mstream_cstr(ms, " as string"); break;
                case ARG_ARRAY: break;
                case ARG_LABEL: break;
                case ARG_DABEL: break;
                case ARG_ANY: break;
                case ARG_USER_DEFINED_VAR_PTR: break;
            }
            arg = arg->next;
        }

        mstream_putc(ms, '%');
        mstream_fmt(ms, "%S", cmd->help, data);
        mstream_putc(ms, '\n');

        cmd = cmd->next;
    }
    return 0;
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
    struct mstream ms = mstream_init_writeable();

    if (parse_cmdline(argc, argv, &cfg) != 0)
        return -1;

    for (int i = 0; i != cfg.input_files_count; ++i)
    {
        struct mfile mf;
        struct root  root = {0};

        /* File names can be empty if CMake uses generator expressions for file
         * names */
        if (!*cfg.input_files[i])
            continue;

        if (mfile_map_read(&mf, cfg.input_files[i], 0) != 0)
            return -1;

        parser_init(&parser, &mf);
        if (parse(&parser, &root, &cfg) != 0)
            return -1;

        switch (cfg.target)
        {
            case TARGET_NONE:
                fprintf(
                    stderr,
                    "Error: No target specified. Use -t <winres|elf>\n");
                return -1;

            case TARGET_WINRES:
                if (gen_winres_resource(&ms) != 0)
                    return -1;
                break;

            case TARGET_ELF:
                if (gen_elf_resource(&ms, &root, parser.data) != 0)
                    return -1;
                break;
        }
    }

    if (write_resource(&ms, cfg.output_file) != 0)
        return -1;

    return 0;
}
