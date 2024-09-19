#include "./ir_internal.hpp"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

int
ir_optimize(struct ir_module* mod)
{
    auto FPM = std::make_unique<llvm::FunctionPassManager>();
    auto LAM = std::make_unique<llvm::LoopAnalysisManager>();
    auto FAM = std::make_unique<llvm::FunctionAnalysisManager>();
    auto CGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    auto MAM = std::make_unique<llvm::ModuleAnalysisManager>();
    auto PIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
    auto SI = std::make_unique<llvm::StandardInstrumentations>(
        mod->ctx, /*DebugLogging*/ true);

    SI->registerCallbacks(*PIC, MAM.get());

    // Add transform passes.
    // Promote allocas to registers.
    FPM->addPass(llvm::PromotePass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    FPM->addPass(llvm::InstCombinePass());
    // Reassociate expressions.
    FPM->addPass(llvm::ReassociatePass());
    // Eliminate Common SubExpressions.
    FPM->addPass(llvm::GVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    FPM->addPass(llvm::SimplifyCFGPass());

    // Register analysis passes used in these transform passes.
    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(*MAM);
    PB.registerFunctionAnalyses(*FAM);
    PB.crossRegisterProxies(*LAM, *FAM, *CGAM, *MAM);

    FPM->run(*F, *FAM);

    return 0;
}
