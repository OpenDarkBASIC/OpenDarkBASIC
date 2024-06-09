#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
}

struct ir_module
{
    ir_module(const char* module_name) : mod(module_name, ctx) {}
    llvm::LLVMContext ctx;
    llvm::Module      mod;
};
