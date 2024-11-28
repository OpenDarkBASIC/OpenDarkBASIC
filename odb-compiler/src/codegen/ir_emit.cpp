#include "./ir_internal.hpp"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/TargetParser/Host.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-util/log.h"
}

int
ir_emit(struct ir_module* ir, const char* filepath)
{
    std::error_code      EC;
    llvm::raw_fd_ostream dest(filepath, EC, llvm::sys::fs::OF_None);
    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return -1;
    }

    llvm::legacy::PassManager pass;
    auto                      FileType = llvm::CodeGenFileType::ObjectFile;
    if (ir->TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        llvm::errs() << "TargetMachine can't emit a file of this type\n";
        return -1;
    }

    pass.run(ir->Mod);
    dest.flush();

    return 0;
}
