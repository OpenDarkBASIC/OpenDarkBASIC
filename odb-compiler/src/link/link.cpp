extern "C" {
#include "odb-compiler/codegen/target.h"
#include "odb-compiler/link/link.h"
#include "odb-util/fs.h"
#include "odb-util/log.h"
#include "odb-util/ospath_list.h"
}

#include "lld/Common/Driver.h"
#include <string>

LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(elf)

static int
link_windows(
    struct ospathc_list* obj_files,
    struct ospathc       output_executable,
    enum target_arch     arch)
{
    llvm::SmallVector<const char*> args;

    args.push_back("lld-link");
    args.push_back("-nodefaultlib");
    args.push_back("-entry:main");
    args.push_back("-subsystem:console");

    switch (arch)
    {
        case TARGET_x86_64: args.push_back("-machine:X64"); break;
        case TARGET_i386: args.push_back("-machine:I386"); break;
        case TARGET_AArch64: args.push_back("-machine:aarch64"); break;
    }

    std::string outNameArg
        = "-out:"
          + std::string(output_executable.str.data, output_executable.len);
    args.push_back(outNameArg.c_str());

    // Win32 API
    // args.push_back("Kernel32.lib");

    // C runtime -- All 3 are needed
    // args.push_back("libucrt.lib");      // /MT of "Universal C-Runtime"
    // args.push_back("libvcruntime.lib"); // /MT of vcruntime
    // args.push_back("libcmt.lib");       // /MT of CRT initialization and
    // termination

    struct ospathc obj_file;
    ospathc_for_each(obj_files, obj_file)
    {
        args.emplace_back(ospathc_cstr(obj_file));
    }

    log_dbg(
        "%s\n",
        [&args]
        {
            std::string s;
            for (const auto& arg : args)
                s += std::string(" ") + arg;
            return s;
        }()
            .c_str());

    if (lld::coff::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;

    return -1;
}

static int
link_linux(
    struct ospathc_list* obj_files,
    struct ospathc       output_executable,
    enum target_arch     arch)
{
    llvm::SmallVector<const char*> args;

    args.push_back("ld.lld");
    args.push_back("--nostdlib");
    args.push_back("--entry=_start");
    args.push_back("--rpath=.:./odb-sdk/plugins");

    switch (arch)
    {
        case TARGET_i386:
            args.push_back("-melf_i386");
            args.push_back("--dynamic-linker=/lib/ld-linux.so.2");
            break;
        case TARGET_x86_64:
            args.push_back("-melf_x86_64");
            args.push_back("--dynamic-linker=/lib64/ld-linux-x86-64.so.2");
            args.push_back("-L/usr/lib64");
            args.push_back("-L/usr/lib/x86_64-linux-gnu");
            break;
        case TARGET_AArch64: args.push_back("-melf_aarch64"); return -1;
    }

    args.push_back("-o");
    args.push_back(ospathc_cstr(output_executable));

    struct ospathc obj_file;
    ospathc_for_each(obj_files, obj_file)
    {
        args.push_back(ospathc_cstr(obj_file));
    }

    if (fs_file_exists(cstr_ospathc("/usr/lib64/crt1.o")))
        args.push_back("/usr/lib64/crt1.o");
    if (fs_file_exists(cstr_ospathc("/usr/lib/x86_64-linux-gnu/crt1.o")))
        args.push_back("/usr/lib/x86_64-linux-gnu/crt1.o");
    args.push_back("-lc");
    args.push_back("-lm");

    log_dbg(
        "%s\n",
        [&args]
        {
            std::string s;
            for (const auto& arg : args)
                s += std::string(" ") + arg;
            return s;
        }()
            .c_str());

    if (lld::elf::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;

    return -1;
}

int
odb_link(
    struct ospathc_list* obj_files,
    struct ospathc       output_executable,
    enum target_arch     arch,
    enum target_platform platform)
{

    switch (platform)
    {
        case TARGET_WINDOWS:
            return link_windows(obj_files, output_executable, arch);
        case TARGET_LINUX:
            return link_linux(obj_files, output_executable, arch);
        case TARGET_MACOS: break;
    }

    return log_err(
        "Linking for platform {quote:%s} is not yet implemented.\n",
        target_platform_to_name(platform));
}
