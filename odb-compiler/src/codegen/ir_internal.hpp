#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace llvm {
class TargetMachine;
}

extern "C" {
#include "odb-compiler/codegen/ir.h"
}

struct ir_module
{
    ir_module(const char* module_name)
        : Mod(module_name, Ctx), TargetMachine(nullptr)
    {
    }

    llvm::LLVMContext          Ctx;
    llvm::Module               Mod;
    llvm::TargetMachine* TargetMachine;
};
