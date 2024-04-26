extern "C" {
#include "odb-compiler/codegen/codegen.h"
}

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

int
odb_codegen(
        union odb_ast_node* program,
        const char* output_name,
        const char* module_name,
        /*enum odb_sdk_type sdkType,*/
        enum odb_codegen_output_type output_type,
        enum odb_codegen_arch arch,
        enum odb_codegen_platform platform)
{
    llvm::LLVMContext ctx;
    llvm::Module mod(module_name, ctx);
    llvm::IRBuilder<> b(ctx);

    // Make the function type:  double(double,double)
    std::vector<llvm::Type*> Integers(2, llvm::Type::getInt32Ty(ctx));
    llvm::FunctionType* FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), Integers, false);
    llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", &mod);

    int i = 0;
    static const char* names[] = { "a", "b" };
    for (auto& arg : F->args())
        arg.setName(names[i++]);

    // Create a new basic block to start insertion into.
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ctx, "entry", F);
    b.SetInsertPoint(BB);

    llvm::Value* lhs = llvm::ConstantInt::get(ctx, llvm::APInt(32, 2));
    llvm::Value* rhs = llvm::ConstantInt::get(ctx, llvm::APInt(32, 3));
    llvm::Value* RetVal = b.CreateAdd(lhs, rhs, "addtmp");
    // Finish off the function.
    b.CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*F);

    mod.print(llvm::outs(), nullptr);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
   
    std::string Error;
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        llvm::errs() << Error;
        return -1;
    }
    
    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::Static);

    mod.setDataLayout(TargetMachine->createDataLayout());
    mod.setTargetTriple(TargetTriple);

    std::error_code EC;
    llvm::raw_fd_ostream dest(output_name, EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return -1;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return -1;
    }

    pass.run(mod);
    dest.flush();

    return 0;
}
