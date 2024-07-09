#include "odb-compiler/sdk/dlparser.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mfile.h"
#include "odb-sdk/utf8.h"
#include <stdio.h>

int
dlparser_symbols_in_section(
    struct ospathc filepath,
    int            (*on_symbol)(const char* sym, void* user),
    void*          user)
{
    return -1;
}

#define READ_LU16(data, off) ((data[(off) + 0] << 0) | (data[(off) + 1] << 8))
#define READ_LU32(data, off)                                                   \
    ((data[(off) + 0] << 0) | (data[(off) + 1] << 8) | (data[(off) + 2] << 16) \
     | (data[(off) + 3] << 24))

static uint32_t
rva_to_offset(const uint8_t* data, int secs_off, int secs_count, uint32_t rva)
{
    int i;
    for (i = 0; i != secs_count; ++i)
    {
        uint32_t sec_off = secs_off + 40 * i;
        uint32_t va = READ_LU32(data, sec_off + 12);
        uint32_t size = READ_LU32(data, sec_off + 16);
        uint32_t data_off = READ_LU32(data, sec_off + 20);

        if (rva >= va && rva < va + size)
            return rva - va + data_off;
    }

    return 0;
}

static void
print_string_resource_entry(
    const uint8_t* data, int secs_off, int secs_count, int off)
{
    uint32_t rva = ((uint8_t)data[off + 0] << 0) | ((uint8_t)data[off + 1] << 8)
                   | ((uint8_t)data[off + 2] << 16)
                   | ((uint8_t)data[off + 3] << 24);
    uint32_t size
        = ((uint8_t)data[off + 4] << 0) | ((uint8_t)data[off + 5] << 8)
          | ((uint8_t)data[off + 6] << 16) | ((uint8_t)data[off + 7] << 24);
    uint32_t codepage
        = ((uint8_t)data[off + 8] << 0) | ((uint8_t)data[off + 9] << 8)
          | ((uint8_t)data[off + 10] << 16) | ((uint8_t)data[off + 11] << 24);

    uint32_t offset = rva_to_offset(data, secs_off, secs_count, rva);
    uint32_t end = offset + size;

    while (offset < end)
    {
        uint32_t len = READ_LU16(data, offset);
        offset += 2;

        if (len == 0)
            continue;

        struct utf16_view utf16 = {(const uint16_t*)(data + offset), len};
        struct utf8       utf8 = empty_utf8();
        if (utf16_to_utf8(&utf8, utf16) == 0)
            printf("%.*s\n", utf8.len, utf8.data);
        else
        {
            log_note("", "offset: 0x%x, size: 0x%x\n", offset, len);
            log_hex_ascii(utf16.data, utf16.len);
        }
        utf8_deinit(utf8);

        offset += len * 2;
    }
}

static int
parse_directory_table(
    const uint8_t* data,
    int            secs_off,
    int            secs_count,
    int            dir_off,
    int            off,
    int            level)
{
    int i;

    /* Directory table header */
    uint32_t characteristics = READ_LU32(data, off + 0);
    uint32_t datetime = READ_LU32(data, off + 4);
    uint16_t major = READ_LU16(data, off + 8);
    uint16_t minor = READ_LU16(data, off + 10);
    uint16_t name_entries = READ_LU16(data, off + 12);
    uint16_t id_entries = READ_LU16(data, off + 14);

    /* Directory entries */
    for (i = 0; i != (int)(name_entries + id_entries); i++)
    {
        uint32_t name_or_id = READ_LU32(data, off + 16 + 8 * i + 0);
        uint32_t offset = READ_LU32(data, off + 16 + 8 * i + 4);

        if (offset & 0x80000000)
        {
            if (level == 1 && name_or_id != 6 /* RT_STRING */)
                continue;
            if (parse_directory_table(
                    data,
                    secs_off,
                    secs_count,
                    dir_off,
                    dir_off + (offset & 0x7FFFFFFF),
                    level + 1)
                != 0)
                return -1;
            continue;
        }

        if (name_or_id & 0x80000000)
        {
            log_sdk_warn("Ignoring name offset\n");
            continue;
        }

        print_string_resource_entry(
            data, secs_off, secs_count, dir_off + offset);
    }

    return 0;
}

static int
parse_resource_section(
    struct ospathc filepath,
    const uint8_t* data,
    int            secs_off,
    int            secs_count,
    int            sec_off)
{
    uint32_t va = READ_LU32(data, sec_off + 12);
    uint32_t size = READ_LU32(data, sec_off + 16);
    uint32_t data_off = READ_LU32(data, sec_off + 20);
    return parse_directory_table(
        data, secs_off, secs_count, data_off, data_off, 1);
}

static int
parse_sections(
    struct ospathc filepath, const uint8_t* data, int secs_off, int secs_count)
{
    int i;
    log_dbg("", "Plugin: {quote:%s}\n", ospathc_cstr(filepath));
    for (i = 0; i != secs_count; ++i)
    {
        int sec_off = secs_off + 40 * i;
        if (memcmp(data + sec_off, ".rsrc\0\0", 8) == 0)
            return parse_resource_section(
                filepath, data, secs_off, secs_count, sec_off);
    }

    return log_sdk_err(
        "No {emph:.rsrc} section found in file {emph:%s}\n",
        ospathc_cstr(filepath));
}

static int
parse_data_directory(
    struct ospathc filepath,
    const uint8_t* data,
    int            data_dir_off,
    int            section_count)
{
    return parse_sections(filepath, data, data_dir_off + 8 * 16, section_count);
}

static int
parse_optional_header32(
    struct ospathc filepath,
    const uint8_t* data,
    int            opt_header32_off,
    int            section_count)
{
    int data_dir = opt_header32_off + 96;
    return parse_data_directory(filepath, data, data_dir, section_count);
}

static int
parse_optional_header64(
    struct ospathc filepath,
    const uint8_t* data,
    int            opt_header64_off,
    int            section_count)
{
    int data_dir = opt_header64_off + 112;
    return parse_data_directory(filepath, data, data_dir, section_count);
}

static int
parse_nt_header(struct ospathc filepath, const uint8_t* data, int off)
{
    uint32_t sig = READ_LU32(data, off);
    uint16_t machine = READ_LU16(data, off + 4);
    uint16_t section_count = READ_LU16(data, off + 6);
    uint32_t datetime = READ_LU32(data, off + 8);
    uint32_t symbols_offset = READ_LU32(data, off + 12);
    uint32_t symbol_count = READ_LU32(data, off + 16);
    uint16_t optional_header_size = READ_LU16(data, off + 20);
    uint16_t characteristics = READ_LU16(data, off + 22);

    uint16_t optional_header_magic = READ_LU16(data, off + 24);

    if (sig != 0x4550)
        return log_sdk_err(
            "Invalid NT signature {quote:0x%x} in file {emph:%s}\n",
            sig,
            ospathc_cstr(filepath));

#if 0
    /* The Windows PE loader ignores this value and instead uses the magic value
     * in the optional header */
    if (machine == 0x8864 /*AMD64*/)
    {
    }
    else if (machine == 0x14C /*i386*/)
    {
        log_dbg("", "i386\n");
    }
    else
        return log_sdk_err("Unsupported machine {emph:0x%x}\n", machine);
#endif
    if (optional_header_magic == 0x10B /*PE32*/)
        return parse_optional_header32(filepath, data, off + 24, section_count);
    if (optional_header_magic == 0x20B /*PE32+*/)
        return parse_optional_header64(filepath, data, off + 24, section_count);

    return log_sdk_err(
        "Unknown OptionalHeader magic {quote:0x%x} in file {emph:%s}\n",
        optional_header_magic,
        ospathc_cstr(filepath));
}

static int
parse_dos_header(struct ospathc filepath, const uint8_t* data, int len)
{
    if (len < 0x40)
        return log_sdk_err(
            "File too small: {emph:%s}\n", ospathc_cstr(filepath));

    uint16_t magic = READ_LU16(data, 0);
    uint16_t nt_off = READ_LU32(data, 0x3C);

    if (magic != 0x5A4D)
        return log_sdk_err(
            "Invalid e_magic {quote:0x%x} in file {emph:%s}\n",
            magic,
            ospathc_cstr(filepath));

    return parse_nt_header(filepath, data, nt_off);
}

int
dlparser_strings(
    struct ospathc filepath,
    int            (*on_string)(const char* sym, void* user),
    void*          user)
{
    struct mfile mf;
    if (mfile_map_read(&mf, filepath, 1) != 0)
        return -1;

    parse_dos_header(filepath, mf.address, mf.size);

    mfile_unmap(&mf);
    return -1;
#if 0
    auto bufferOrError = llvm::MemoryBuffer::getFile(
        llvm::StringRef(ospathc_cstr(path), path.len));
    if (!bufferOrError)
        return nullptr;
    const auto& buffer = bufferOrError.get();

    auto objectOrError
        = llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
    if (!objectOrError)
        return nullptr;
    const auto& object = *objectOrError.get();

    if (!object.isCOFF())
        return nullptr;

    const llvm::object::COFFObjectFile& coffFile
        = static_cast<const llvm::object::COFFObjectFile&>(object);
    for (const llvm::object::SectionRef& section : coffFile.sections())
    {
        auto name = section.getName();
        if (!name)
            return nullptr;
        if (name.get() == ".rsrc")
        {
            auto sectionData = section.getContents();
            if (!sectionData)
                return nullptr;

            coffFile.getRvaPtr();

            parse_directory_table(sectionData->data(), 0, 1);
        }
    }

    return nullptr;
#endif
}
