#if defined(_WIN32)
#define NL "\r\n"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#define NL "\n"
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------------------
 * Printing and formatting
 * ------------------------------------------------------------------------- */

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
 * Settings & Command Line
 * ------------------------------------------------------------------------- */

struct cfg
{
    const char* suite_name;
    const char* output_file;
    char**      input_files;
    int         input_file_count;
};

static int
parse_cmdline(int argc, char** argv, struct cfg* cfg)
{
    int i;
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--suite") == 0)
        {
            if (i + 1 >= argc)
                return print_error("Missing suite name. Use --suite <name>\n");

            cfg->suite_name = argv[++i];
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if (i + 1 >= argc)
                return print_error(
                    "Missing input files. Use -i <file1> [file2...]\n");

            cfg->input_files = &argv[++i];
            cfg->input_file_count = 1;
            for (i++; i != argc; ++i)
            {
                if (argv[i][0] == '-')
                {
                    i--;
                    break;
                }
                cfg->input_file_count++;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 >= argc)
                return print_error(
                    "Missing output file name. Use -o <output.cpp>\n");

            cfg->output_file = argv[++i];
        }
    }

    if (cfg->suite_name == NULL)
        return print_error("No suite name was specified. Use --suite <name>\n");

    if (cfg->output_file == NULL)
        return print_error(
            "No output file name was specified. Use -o <output.cpp>\n");

    if (cfg->input_files == NULL)
        return print_error(
            "No input files were specified. Use -i <file1> [file2...]\n");

    return 0;
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
 * \param[in] filepath Utf8 encoded file path.
 * \param[in] silence_open_error If zero, an error is printed to stderr if
 * mapping fails. If one, no errors are printed. When comparing the generated
 * code with an already existing file, we allow the function to fail silently if
 * the file does not exist. \return Returns 0 on success, negative on failure.
 */
static int
mfile_map_read(struct mfile* mf, const char* filepath, int silence_open_error)
{
#if defined(WIN32)
    HANDLE        hFile;
    LARGE_INTEGER liFileSize;
    HANDLE        mapping;
    wchar_t*      utf16_filepath;

    utf16_filepath = utf8_to_utf16(filepath, (int)strlen(filepath));
    if (utf16_filepath == NULL)
        goto utf16_conv_failed;

    /* Try to open the file */
    hFile = CreateFileW(
        utf16_filepath, /* File name */
        GENERIC_READ,   /* Read only */
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
    utf_free(utf16_filepath);

    mf->size = (int)liFileSize.QuadPart;

    return 0;

map_view_failed:
    CloseHandle(mapping);
create_file_mapping_failed:
get_file_size_failed:
    CloseHandle(hFile);
open_failed:
    utf_free(utf16_filepath);
utf16_conv_failed:
    return -1;
#else
    struct stat stbuf;
    int         fd;

    fd = open(filepath, O_RDONLY);
    if (fd < 0)
    {
        if (!silence_open_error)
            print_error(
                "Failed to open file \"%s\": %s\n", filepath, strerror(errno));
        goto open_failed;
    }

    if (fstat(fd, &stbuf) != 0)
    {
        print_error(
            "Failed to stat file \"%s\": %s\n", filepath, strerror(errno));
        goto fstat_failed;
    }
    if (!S_ISREG(stbuf.st_mode))
    {
        print_error("File \"%s\" is not a regular file!\n", filepath);
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
            "Failed to mmap() file \"%s\": %s\n", filepath, strerror(errno));
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
 * \param[in] filepath Utf8 encoded file path.
 * \param[in] size Size of the file in bytes. This is used to allocate space
 * on the file system.
 * \return Returns 0 on success, negative on failure.
 */
static int
mfile_map_write(struct mfile* mf, const char* filepath, int size)
{
#if defined(WIN32)
    HANDLE   hFile;
    HANDLE   mapping;
    wchar_t* utf16_filepath;

    utf16_filepath = utf8_to_utf16(filepath, (int)strlen(filepath));
    if (utf16_filepath == NULL)
        goto utf16_conv_failed;

    /* Try to open the file */
    hFile = CreateFileW(
        utf16_filepath,               /* File name */
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
    utf_free(utf16_filepath);

    mf->size = size;

    return 0;

map_view_failed:
    CloseHandle(mapping);
create_file_mapping_failed:
    CloseHandle(hFile);
open_failed:
    utf_free(utf16_filepath);
utf16_conv_failed:
    return -1;
#else
    int fd = open(
        filepath,
        O_CREAT | O_RDWR | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        print_error(
            "Failed to open file \"%s\" for writing: %s\n",
            filepath,
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
            filepath,
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
            filepath,
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

static int
fs_list_files_recurse(
    const char* path,
    int   (*on_entry)(const char* filepath, const char* filename, void* user),
    void* user)
{
#if defined(WIN32)
    DWORD           dwError;
    WIN32_FIND_DATA ffd;
    char*           path_search;
    char*           filepath = NULL;
    int             path_len = strlen(path);
    int             ret = 0;
    HANDLE          hFind = INVALID_HANDLE_VALUE;

    path_search = malloc(path_len + 3);
    if (path_search == NULL)
        goto str_set_failed;
    strcpy(path_search, path);
    if (path_search[path_len - 1] == '/')
        path_search[path_len - 1] = '\\';
    if (path_search[path_len - 1] != '\\')
        strcat(path_search, "\\");
    strcat(path_search, "*");

    hFind = FindFirstFileA(path_search, &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        goto first_file_failed;

    do
    {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;

        filepath = realloc(filepath, strlen(path) + strlen(ffd.cFileName) + 2);
        strcpy(filepath, path);
        if (filepath[strlen(filepath) - 1] != '/' && filepath[strlen(filepath) - 1] != '\\')
            strcat(filepath, "\\");
        strcat(filepath, ffd.cFileName);

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            ret = on_entry(filepath, ffd.cFileName, user);
            if (ret != 0)
                goto out;
        }
        else if(ffd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
        {
            ret = fs_list_files_recurse(filepath, on_entry, user);
            if (ret)
                return ret;
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
        ret = -1;

out:
    FindClose(hFind);
first_file_failed:
    free(path_search);
str_set_failed:
    return ret;
#else
    DIR*                 dp;
    const struct dirent* ep;
    char*                filepath = NULL;
    int                  ret = 0;

    dp = opendir(path);
    if (!dp)
        goto first_file_failed;

    while ((ep = readdir(dp)) != NULL)
    {
        unsigned char d_type;
        if (ep->d_name[0] == '.')
            continue;

        filepath = realloc(filepath, strlen(path) + strlen(ep->d_name) + 2);
        strcpy(filepath, path);
        if (filepath[strlen(filepath) - 1] != '/')
            strcat(filepath, "/");
        strcat(filepath, ep->d_name);

        d_type = ep->d_type;
        if (d_type == DT_UNKNOWN)
        {
            struct stat st;
            if (stat(filepath, &st))
            {
                ret = -1;
                goto out;
            }

            if (S_ISREG(st.st_mode))
                d_type = DT_REG;
            else if (S_ISDIR(st.st_mode))
                d_type = DT_DIR;
        }

        switch (d_type)
        {
            case DT_REG:
                ret = on_entry(filepath, ep->d_name, user);
                if (ret != 0)
                    goto out;
                break;

            case DT_DIR:
                ret = fs_list_files_recurse(filepath, on_entry, user);
                if (ret)
                    return ret;
                break;
        }
    }

out:
    if (filepath)
        free(filepath);
    closedir(dp);
first_file_failed:
    return ret;
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

static inline void
mstream_str(struct mstream* ms, const char* str, int len)
{
    mstream_grow(ms, len);
    memcpy((char*)ms->address + ms->write_ptr, str, len);
    ms->write_ptr += len;
}

/*! Write a C-string to the mstream buffer */
static inline void
mstream_cstr(struct mstream* ms, const char* cstr)
{
    mstream_str(ms, cstr, (int)strlen(cstr));
}

/*!
 * \brief Write a formatted string to the mstream buffer.
 * This function is similar to printf() but only implements a subset of the
 * format specifiers. These are:
 *   %i - Write an integer (int)
 *   %d - Write an integer (int)
 *   %s - Write a c-string (const char*)
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
                    const char* str = va_arg(va, const char*);
                    int         len = va_arg(va, int);
                    mstream_str(ms, str, len);
                    continue;
                }
            }
        mstream_putc(ms, fmt[i]);
    }
    va_end(va);
}

/* -----------------------------------------------------------------------------
 * Container for matching source and output files
 * -------------------------------------------------------------------------- */

struct ci_file
{
    char* dbaname;
    char* dbapath;
    char* outpath;
};

struct ci_files
{
    int            count;
    struct ci_file file[1];
};

static struct ci_files empty_ci_files;

static char*
cstr_dup(const char* cstr)
{
    char* s = malloc(strlen(cstr) + 1);
    strcpy(s, cstr);
    return s;
}

static int
cstr_ends_with(const char* cstr, const char* cmp)
{
    int len = strlen(cstr);
    int cmp_len = strlen(cmp);
    if (len < cmp_len)
        return 0;

    return memcmp(cstr + len - cmp_len, cmp, (size_t)cmp_len) == 0;
}

static int
cstr_equal_without_ext(const char* s1, const char* s2)
{
    for (;;)
    {
        if (*s1 == '.' && *s1 == '.')
            return 1;
        if (*s1 != *s2)
            return 0;
        if (!*s1 || !*s2)
            return 1;
        s1++, s2++;
    }
}

static void
ci_files_grow(struct ci_files** files)
{
    int new_count = (*files)->count + 1;
    *files = realloc(
        *files == &empty_ci_files ? NULL : *files, new_count * sizeof(**files));
    (*files)->count = new_count;
}

/*! Find corresponding out file from an dba file */
static int
ci_files_find_out(const struct ci_files* files, const char* dbapath)
{
    int i;
    for (i = 0; i != files->count; ++i)
        if (cstr_equal_without_ext(files->file[i].outpath, dbapath))
            return i;

    return files->count;
}

/*! Find corresponding dba file from an out file */
static int
ci_files_find_dba(const struct ci_files* files, const char* outpath)
{
    int i;
    for (i = 0; i != files->count; ++i)
        if (cstr_equal_without_ext(files->file[i].dbapath, outpath))
            return i;

    return files->count;
}

static void
ci_files_add_dba(struct ci_files** files, const char* dbapath)
{
    char* name;
    char* path;
    int   i = ci_files_find_out(*files, dbapath);
    if (i == (*files)->count)
    {
        ci_files_grow(files);
        (*files)->file[i].outpath = "";
    }

    path = name = cstr_dup(dbapath);
    name += strlen(name);
    while (name != path)
    {
        if (*name == '/' || *name == '\\')
        {
            name++;
            break;
        }
        name--;
    }

    (*files)->file[i].dbapath = path;
    (*files)->file[i].dbaname = name;
}

static void
ci_files_add_out(struct ci_files** files, const char* outpath)
{
    int i = ci_files_find_dba(*files, outpath);
    if (i == (*files)->count)
    {
        ci_files_grow(files);
        (*files)->file[i].dbaname = "";
        (*files)->file[i].dbapath = "";
    }

    (*files)->file[i].outpath = cstr_dup(outpath);
}

static int
collect_ci_files(const struct cfg* cfg, struct ci_files** files)
{
    int i;
    for (i = 0; i != cfg->input_file_count; ++i)
    {
        if (cstr_ends_with(cfg->input_files[i], ".dba"))
            ci_files_add_dba(files, cfg->input_files[i]);

        if (cstr_ends_with(cfg->input_files[i], ".out"))
            ci_files_add_out(files, cfg->input_files[i]);
    }

    for (i = 0; i != (*files)->count; ++i)
    {
        if (!*(*files)->file[i].outpath)
            return print_error(
                "Missing out file for dba file `%s`\nEach .out file needs to "
                "have a corresponding .dba file with the same name containing "
                "the source code of the test case. The result of this program "
                "is compared with the contents of the .out file.\n",
                (*files)->file[i].dbapath);
        if (!*(*files)->file[i].dbapath)
            return print_error(
                "Missing dba file for out file `%s`\nEach .dba test case "
                "needs to have a corresponding .out file with the same name "
                "containing the expected output "
                "f the program.\n",
                (*files)->file[i].outpath);
    }

    return 0;
}

/* -----------------------------------------------------------------------------
 * Code generation
 * -------------------------------------------------------------------------- */

static int
gen_source(
    struct mstream* ms, const struct ci_files* files, const struct cfg* cfg)
{
    int f;
    
#if defined(_WIN32)
    mstream_cstr(ms, "#define WIN32_LEAN_AND_MEAN" NL);
    mstream_cstr(ms, "#include <Windows.h>" NL);
#endif

    mstream_cstr(ms, "#include <gmock/gmock.h>" NL);
    mstream_cstr(ms, "#include \"odb-sdk/tests/Utf8Helper.hpp\"" NL NL);
    mstream_cstr(ms, "extern \"C\" {" NL);
    mstream_cstr(ms, "#include \"odb-sdk/process.h\"" NL);
    mstream_cstr(ms, "#include \"odb-sdk/utf8.h\"" NL);
    mstream_cstr(ms, "}" NL NL);

    mstream_fmt(ms, "#define NAME %s" NL NL, cfg->suite_name);

    mstream_cstr(ms, "using namespace testing;" NL NL);

    mstream_cstr(ms, "struct NAME : Test" NL);
    mstream_cstr(ms, "{" NL);
    mstream_cstr(ms, "    void SetUp() override" NL);
    mstream_cstr(ms, "    {" NL);
#if defined(_WIN32)
    mstream_cstr(ms, "        CreateDirectoryA(\"ci-tests\", NULL);" NL);
#else
    mstream_cstr(ms, "        mkdir(\"ci-tests\", 0755);" NL);
#endif
    mstream_cstr(ms, "        out = empty_utf8();" NL);
    mstream_cstr(ms, "        err = empty_utf8();" NL);
    mstream_cstr(ms, "    }" NL);
    mstream_cstr(ms, "    void TearDown() override" NL);
    mstream_cstr(ms, "    {" NL);
    mstream_cstr(ms, "        utf8_deinit(out);" NL);
    mstream_cstr(ms, "        utf8_deinit(err);" NL);
    mstream_cstr(ms, "    }" NL);
    mstream_cstr(ms, "    struct utf8 out, err;" NL);
    mstream_cstr(ms, "};" NL NL);

    for (f = 0; f != files->count; ++f)
    {
        int          i;
        int          len;
        char*        name;
        struct mfile mfdba, mfout;
        if (mfile_map_read(&mfdba, files->file[f].dbapath, 0) != 0)
            return -1;
        if (mfile_map_read(&mfout, files->file[f].outpath, 0) != 0)
            return -1;

        /* Remove .dba extension from name */
        len = strlen(files->file[f].dbaname);
        name = files->file[f].dbaname;
        while (len--)
            if (name[len] == '.')
            {
                name[len] = '\0';
                break;
            }

        /* Replace spaces etc. with underscores */
        name = files->file[f].dbaname;
        for (; *name; name++)
        {
            if (*name >= 'a' && *name <= 'z' || *name >= 'A' && *name <= 'Z'
                || *name >= '0' && *name <= '9' || *name == '_')
                continue;
            *name = '_';
        }

        /* Call odb-cli to compile the program */
        mstream_fmt(ms, "TEST_F(NAME, %s)" NL, files->file[f].dbaname);
        mstream_cstr(ms, "{" NL);
        /* clang-format off */
        mstream_fmt(
            ms,
            "    const char* compile_argv[] = {" NL
#if defined(_WIN32)
            "        \"odb-cli.exe\"," NL
#else
            "        \"./odb-cli\"," NL
#endif
            "        \"-b\"," NL
            "        \"--dba\"," NL
            "        \"--output\"," NL
#if defined(_WIN32)
            "        \"ci-tests\\\\%s.exe\"," NL
#else
            "        \"ci-tests/%s\"," NL
#endif
            "        NULL" NL
            "    };" NL,
            files->file[f].dbaname);
        mstream_cstr(ms,
            "    ASSERT_THAT(process_run(" NL
#if defined(_WIN32)
            "        cstr_ospathc(\"odb-cli.exe\")," NL
#else
            "        cstr_ospathc(\"./odb-cli\")," NL
#endif
            "        compile_argv," NL
            "        cstr_utf8_view(" NL
            "            \"");
        /* clang-format on */
        for (i = 0; i != mfdba.size; ++i)
        {
            char c = ((char*)mfdba.address)[i];
            if (c == '"' || c == '\\')
                mstream_putc(ms, '\\');
            if (c == '\n')
                mstream_cstr(ms, "\\n\"" NL "            \"");
            else if (c == '\r')
                continue;
            else
                mstream_putc(ms, c);
        }
        mstream_cstr(ms, "\")," NL);
        mstream_cstr(
            ms,
            "        &out, &err, 3000), Eq(0)) << std::string(err.data, "
            "err.len);" NL);
        mstream_cstr(ms, "    ASSERT_THAT(out, Utf8Eq(\"\"));" NL);
        /* Compiler will generate output on stderr, but it's hard to predict
         * what it will be exactly */
        /*mstream_cstr(ms, "    ASSERT_THAT(err, Utf8Eq(\"\"));" NL NL);*/

        /* Run compiled program and assert its output */
        /* clang-format off */
        mstream_fmt(
            ms,
            "    const char* run_argv[] = {" NL
#if defined(_WIN32)
            "        \"ci-tests\\\\%s.exe\"," NL
#else
            "        \"./ci-tests/%s\"," NL
#endif
            "        NULL" NL
            "    };" NL,
            files->file[f].dbaname);
        mstream_fmt(ms,
            "    ASSERT_THAT(process_run(" NL
#if defined(_WIN32)
            "        cstr_ospathc(\"ci-tests\\\\%s.exe\")," NL
#else
            "        cstr_ospathc(\"./ci-tests/%s\")," NL
#endif
            "        run_argv," NL
            "        empty_utf8_view()," NL
            "        &out, &err, 3000), Eq(0));" NL,
            files->file[f].dbaname);
        /* clang-format on */
        mstream_fmt(
            ms,
            "    ASSERT_THAT(out, Utf8Eq(%S));" NL,
            (const char*)mfout.address,
            (int)mfout.size);
        /* odb-sdk will print out a memory report if it was built with this
         * enabled */
        /*mstream_cstr(ms, "    ASSERT_THAT(err, Utf8Eq(\"\"));" NL);*/
        mstream_cstr(ms, "}" NL NL);
    }

    return 0;
}

static int
write_to_file(const struct mstream* ms, const char* filename)
{
    struct mfile mf;

    /* Don't write if it is identical to the existing one -- causes less
     * rebuilds */
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
    struct cfg       cfg = {0};
    struct ci_files* files = &empty_ci_files;
    struct mstream   ms = mstream_init_writeable();

    if (!stream_is_terminal(stderr))
        disable_colors = 1;

    if (parse_cmdline(argc, argv, &cfg) != 0)
        return -1;

    if (collect_ci_files(&cfg, &files) != 0)
        return -1;

    if (gen_source(&ms, files, &cfg) != 0)
        return -1;

    return write_to_file(&ms, cfg.output_file);
}
