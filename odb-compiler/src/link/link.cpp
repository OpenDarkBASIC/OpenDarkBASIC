extern "C" {
#include "odb-compiler/link/link.h"
#include "odb-sdk/fs.h"
}

#include "odb-compiler/codegen/codegen.h"
#include "lld/Common/Driver.h"
#include <string>

LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(elf)

static int
link_windows(
    const char*           objs[],
    int                   count,
    const char*           output_name,
    enum odb_codegen_arch arch)
{
    llvm::SmallVector<const char*> args;

    args.push_back("lld-link");
    args.push_back("-nodefaultlib");
    args.push_back("-entry:main");
    args.push_back("-subsystem:console");

    switch (arch)
    {
        case ODB_CODEGEN_x86_64: args.push_back("-machine:X64"); break;
        case ODB_CODEGEN_i386: args.push_back("-machine:I386"); break;
        case ODB_CODEGEN_AArch64: args.push_back("-machine:aarch64"); break;
    }

    std::string outNameArg = "-out:" + std::string(output_name);
    args.push_back(outNameArg.c_str());

    args.push_back("kernel32.lib");

    args.push_back("./odb-sdk/plugins/core-commands.lib");
    args.push_back("./odb-sdk/plugins/test-plugin.lib");

    for (int i = 0; i != count; ++i)
        args.push_back(objs[i]);

    if (lld::coff::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;

    return -1;
}

static int
link_linux(
    const char*           objs[],
    int                   count,
    const char*           output_name,
    enum odb_codegen_arch arch)
{
    llvm::SmallVector<const char*> args;

    args.push_back("ld.lld");
    args.push_back("--nostdlib");
    // args.push_back("--entry=main");
    args.push_back("--rpath=./lib:./odb-sdk/plugins");

    switch (arch)
    {
        case ODB_CODEGEN_i386:
            args.push_back("-melf_i386");
            args.push_back("--dynamic-linker=/lib/ld-linux.so.2");
            break;
        case ODB_CODEGEN_x86_64:
            args.push_back("-melf_x86_64");
            args.push_back("--dynamic-linker=/lib64/ld-linux-x86-64.so.2");
            args.push_back("-L/usr/lib64");
            args.push_back("-L/usr/lib/x86_64-linux-gnu");
            break;
        case ODB_CODEGEN_AArch64: args.push_back("-melf_aarch64"); return -1;
    }

    args.push_back("-o");
    args.push_back(output_name);

    for (int i = 0; i != count; ++i)
        args.push_back(objs[i]);

    args.push_back("./odb-sdk/plugins/core-commands.so");
    // args.push_back("./odb-sdk/plugins/test-plugin.so");

    args.push_back("-L./lib");
    args.push_back("-lodb-sdk");

    if (fs_file_exists(cstr_ospathc("/usr/lib64/crt1.o")))
        args.push_back("/usr/lib64/crt1.o");
    if (fs_file_exists(cstr_ospathc("/usr/lib/x86_64-linux-gnu/crt1.o")))
        args.push_back("/usr/lib/x86_64-linux-gnu/crt1.o");
    args.push_back("-lc");
    args.push_back("-lm");

    if (lld::elf::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;

    return -1;
}

int
odb_link(
    const char* objs[],
    int         count,
    const char* output_name,
    /*enum odb_sdk_type sdkType,*/
    enum odb_codegen_arch     arch,
    enum odb_codegen_platform platform)
{

    switch (platform)
    {
        case ODB_CODEGEN_WINDOWS:
            return link_windows(objs, count, output_name, arch);
        case ODB_CODEGEN_LINUX:
            return link_linux(objs, count, output_name, arch);
        case ODB_CODEGEN_MACOS: break;
    }

    return -1;
}
