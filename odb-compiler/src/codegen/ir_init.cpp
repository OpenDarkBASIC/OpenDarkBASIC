#include "./ir_internal.hpp"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

int
ir_global_init()
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    return 0;
}

void
ir_global_deinit()
{
    llvm::llvm_shutdown();
}

