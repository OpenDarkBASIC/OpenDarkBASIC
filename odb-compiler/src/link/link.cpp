extern "C" {
#include "odb-compiler/link/link.h"
}

#include "lld/Common/Driver.h"
#include <string>

LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(elf)

int odb_link(
    const char* objs[], int count,
    const char* output_name,
    /*enum odb_sdk_type sdkType,*/
    enum odb_codegen_arch arch,
    enum odb_codegen_platform platform)
{
    llvm::SmallVector<const char*> args;
    std::string outNameArg = "-out:" + std::string(output_name);

    args.push_back("lld-link.exe");
    args.push_back("-nodefaultlib");
    args.push_back("-machine:x64");
    args.push_back("-entry:main");
    args.push_back("-subsystem:console");
    args.push_back(outNameArg.c_str());

    for (int i = 0; i != count; ++i)
        args.push_back(objs[i]);

    if (lld::coff::link(args, llvm::outs(), llvm::errs(), false, false))
        return 0;
    return -1;
}
