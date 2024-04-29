#include "odb-compiler/codegen/codegen.h"
extern "C" {
#include "odb-compiler/link/link.h"
}

#include "lld/Common/Driver.h"
#include <string>

LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(elf)

static int
link_windows(
    const char* objs[], int count,
    const char* output_name,
    enum odb_codegen_arch arch)
{
    llvm::SmallVector<const char*> args;

    args.push_back("lld-link");
    args.push_back("-nodefaultlib");
    args.push_back("-entry:main");
    args.push_back("-subsystem:console");
    std::string outNameArg = "-out:" + std::string(output_name);
    args.push_back(outNameArg.c_str());
    
    switch (arch) {
        case ODB_CODEGEN_x86_64  : args.push_back("-machine:x64"); break;
        case ODB_CODEGEN_i386    : args.push_back("-machine:x32"); break;
        case ODB_CODEGEN_AArch64 : args.push_back("-machine:aarch64"); break;
    }

    for (int i = 0; i != count; ++i)
        args.push_back(objs[i]);

    if (lld::coff::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;

    return -1;
}

static int
link_linux(
    const char* objs[], int count,
    const char* output_name,
    enum odb_codegen_arch arch)
{
    llvm::SmallVector<const char*> args;

    args.push_back("ld.lld");
    args.push_back("--nostdlib");
    args.push_back("--entry=main");
    args.push_back("-o");
    args.push_back(output_name);

    args.push_back("-m");
    switch (arch) {
        case ODB_CODEGEN_x86_64  : args.push_back("elf_x86_64"); break;
        case ODB_CODEGEN_i386    : args.push_back("elf_i386"); break;
        case ODB_CODEGEN_AArch64 : args.push_back("elf_aarch64"); break;
    }

    for (int i = 0; i != count; ++i)
        args.push_back(objs[i]);

    if (lld::elf::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;

    return -1;
}

int
odb_link(
    const char* objs[], int count,
    const char* output_name,
    /*enum odb_sdk_type sdkType,*/
    enum odb_codegen_arch arch,
    enum odb_codegen_platform platform)
{

    switch (platform) {
        case ODB_CODEGEN_WINDOWS:
            return link_windows(objs, count, output_name, arch);
        case ODB_CODEGEN_LINUX: 
            return link_linux(objs, count, output_name, arch);
        case ODB_CODEGEN_MACOS:
            break;
    }

    return -1;
}

