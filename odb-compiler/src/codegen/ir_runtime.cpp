#include "./ir_internal.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
}

static int
gen_init_func(
    struct ir_module*    ir,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform)
{
    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ir->ctx),
            {},
            /* isVarArg */ false),
        llvm::Function::ExternalLinkage,
        "odbruntime_init",
        ir->mod);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->ctx, "", F);
    llvm::IRBuilder<> b(BB);

    if (platform == TARGET_WINDOWS)
    {
        llvm::Constant* rpath_data = llvm::ConstantDataArray::getString(
            ir->ctx,
            llvm::StringRef("odb-sdk\\plugins\\"),
            /* Add NULL */ true);
        llvm::GlobalVariable* rpath_const = new llvm::GlobalVariable(
            ir->mod,
            rpath_data->getType(),
            /*isConstant*/ true,
            llvm::GlobalValue::PrivateLinkage,
            rpath_data,
            ".rpath");
        rpath_const->setAlignment(llvm::Align::Constant<1>());

        llvm::Function* rpath_func = llvm::Function::Create(
            llvm::FunctionType::get(
                llvm::Type::getInt32Ty(ir->ctx),
                {llvm::PointerType::get(llvm::Type::getInt8Ty(ir->ctx), 0)},
                false),
            llvm::Function::ExternalLinkage,
            "SetDllDirectoryA",
            &ir->mod);

        b.SetInsertPoint(BB->getFirstNonPHI());
        b.CreateCall(rpath_func, rpath_const);
    }

    // Plugins use odb-sdk. Have to call the global init functions
    if (sdk_type == SDK_ODB)
    {
        llvm::Function* FSDKInit = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(ir->ctx), {}, false),
            llvm::Function::ExternalLinkage,
            "odbsdk_init",
            ir->mod);
        b.CreateCall(FSDKInit, {});
    }

    b.CreateRet(llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0)));

    return 0;
}

static int
gen_deinit_func(
    struct ir_module*    ir,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform)
{
    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ir->ctx),
            {},
            /* isVarArg */ false),
        llvm::Function::ExternalLinkage,
        "odbruntime_deinit",
        ir->mod);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->ctx, "", F);
    llvm::IRBuilder<> b(BB);

    // Plugins use odb-sdk. Have to call the global init functions
    if (sdk_type == SDK_ODB)
    {
        llvm::Function* FSDKDeInit = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
            llvm::Function::ExternalLinkage,
            "odbsdk_deinit",
            ir->mod);
        b.CreateCall(FSDKDeInit, {});
    }

    b.CreateRet(llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0)));

    return 0;
}

int
ir_create_runtime(
    struct ir_module*    ir,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform)
{
    if (gen_init_func(ir, sdk_type, arch, platform) < 0)
        return -1;
    if (gen_deinit_func(ir, sdk_type, arch, platform) < 0)
        return -1;

    ir->mod.print(llvm::outs(), nullptr);

    return 0;
}
