#include "./ir_internal.hpp"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

extern "C" {
#include "odb-util/log.h"
#include "odb-util/mem.h"
}

struct ir_module*
ir_alloc_module(
    const char*          module_name,
    enum target_arch     arch,
    enum target_platform platform)
{
    std::string         Error;
    llvm::TargetOptions Opt;
    llvm::Reloc::Model  Reloc;
    const llvm::Target* Target;
    struct ir_module*   ir;

    const char* CPU = "generic";
    const char* Features = "";
    const char* triple = target_arch_platform_to_triplet(arch, platform);
    Reloc = llvm::Reloc::Static;
    // https://discourse.llvm.org/t/llvm-emitting-wrong-machine-code-for-x64-msvc/81226/1
    if (platform == TARGET_WINDOWS)
        Reloc = llvm::Reloc::PIC_;
    log_dbg("triple: %s, CPU: %s, features: %s\n", triple, CPU, Features);

    Target = llvm::TargetRegistry::lookupTarget(triple, Error);
    if (!Target)
    {
        log_err("Target not found in registry: {quote:%s}\n", triple);
        return nullptr;
    }

    ir = new ir_module(module_name);
    mem_track_allocation(ir);

    ir->TargetMachine
        = Target->createTargetMachine(triple, CPU, Features, Opt, Reloc);
    ir->Mod.setDataLayout(ir->TargetMachine->createDataLayout());
    ir->Mod.setTargetTriple(triple);

    return ir;
}

void
ir_free_module(struct ir_module* ir)
{
    mem_track_deallocation(ir);
    delete ir;
}
