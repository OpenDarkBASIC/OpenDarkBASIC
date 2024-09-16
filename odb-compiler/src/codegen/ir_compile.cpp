#include "./ir_internal.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-util/log.h"
}

int
ir_compile(
    struct ir_module*    ir,
    const char*          filepath,
    enum target_arch     arch,
    enum target_platform platform)
{
    /* clang-format off */
    static const char* target_triples[3][3] = {
        {"i386-pc-windows-msvc", "x86_64-pc-windows-msvc", ""},
        {"i386-linux-gnu",       "x86_64-linux-gnu", ""},
        {"i386-",                "x86_64-", ""}
    };
    /* clang-format on */

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string Error;
    auto TargetTriple = target_triples[platform][arch];
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target)
    {
        llvm::errs() << Error;
        return -1;
    }

    auto                CPU = "generic";
    auto                Features = "";
    llvm::TargetOptions opt;
    auto                TargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, llvm::Reloc::Static);

    log_dbg("[codegen] ", "triple: %s, CPU: %s, features: %s\n", target_triples[platform][arch], CPU, Features);

    ir->mod.setDataLayout(TargetMachine->createDataLayout());
    ir->mod.setTargetTriple(TargetTriple);

    std::error_code      EC;
    llvm::raw_fd_ostream dest(filepath, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return -1;
    }

    llvm::legacy::PassManager pass;
    auto                      FileType = llvm::CodeGenFileType::ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        llvm::errs() << "TargetMachine can't emit a file of this type\n";
        return -1;
    }

    pass.run(ir->mod);
    dest.flush();

    return 0;
}
