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
ir_optimize(struct ir_module* ir)
{
    // Create the analysis managers.
    // These must be declared in this order so that they are destroyed in the
    // correct order due to inter-analysis-manager references.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionPassManager FPM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;
    llvm::ModulePassManager MPM;

    llvm::PassInstrumentationCallbacks PIC;
    llvm::StandardInstrumentations SI(ir->ctx, /*DebugLogging*/ true);
    SI.registerCallbacks(PIC, &MAM);

    // Add transform passes.
    // Promote allocas to registers.
    FPM.addPass(llvm::PromotePass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    FPM.addPass(llvm::InstCombinePass());
    // Reassociate expressions.
    FPM.addPass(llvm::ReassociatePass());
    // Eliminate Common SubExpressions.
    FPM.addPass(llvm::GVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    FPM.addPass(llvm::SimplifyCFGPass());

    // Register all the basic analyses with the managers.
    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
    MPM.run(ir->mod, MAM);

    return 0;
}
