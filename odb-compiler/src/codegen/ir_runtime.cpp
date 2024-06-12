#include "./ir_internal.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/sdk/sdk_type.h"
}

static int
gen_init_dbpro(
    struct ir_module*    ir,
    llvm::BasicBlock*    BB,
    enum target_arch     arch,
    enum target_platform platform)
{
    return 0;
}

static int
gen_deinit_dbpro(
    struct ir_module*    ir,
    llvm::BasicBlock*    BB,
    enum target_arch     arch,
    enum target_platform platform)
{
    return 0;
}

static int
gen_init_odb(
    struct ir_module*    ir,
    llvm::BasicBlock*    BB,
    enum target_arch     arch,
    enum target_platform platform)
{
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
        b.CreateCall(rpath_func, rpath_const);
    }

    // Plugins use odb-sdk. Have to call the global init functions
    llvm::Function* FSDKInit = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "odbsdk_init",
        ir->mod);
    b.CreateCall(FSDKInit, {});

    return 0;
}

static int
gen_deinit_odb(
    struct ir_module*    ir,
    llvm::BasicBlock*    BB,
    enum target_arch     arch,
    enum target_platform platform)
{
    llvm::IRBuilder<> b(BB);

    // Plugins use odb-sdk. Have to call the global init functions
    llvm::Function* FSDKDeInit = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "odbsdk_deinit",
        ir->mod);
    b.CreateCall(FSDKDeInit, {});

    return 0;
}

int
ir_create_runtime(
    struct ir_module*     ir,
    const struct cmd_ids* used_cmds,
    const char*           main_dba_name,
    enum sdk_type         sdk_type,
    enum target_arch      arch,
    enum target_platform  platform)
{
    llvm::Function* F = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ir->ctx),
            {llvm::Type::getInt32Ty(ir->ctx),
             llvm::PointerType::getUnqual(
                 llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(ir->ctx)))},
            /*isVarArg=*/false),
        llvm::Function::ExternalLinkage,
        "main",
        ir->mod);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->ctx, "", F);
    llvm::IRBuilder<> b(BB);

    llvm::Function* FDLOpen = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::PointerType::getUnqual(ir->ctx),
            {llvm::PointerType::getUnqual(ir->ctx),
             llvm::Type::getInt32Ty(ir->ctx)},
            /*isVarArg=*/false),
        llvm::Function::ExternalLinkage,
        "dlopen",
        ir->mod);
    llvm::Function* FDLSym = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::PointerType::getUnqual(ir->ctx),
            {llvm::PointerType::getUnqual(ir->ctx),
             llvm::PointerType::getUnqual(ir->ctx)},
            /*isVarArg=*/false),
        llvm::Function::ExternalLinkage,
        "dlsym",
        ir->mod);

    llvm::GlobalVariable* GVprint_str_func = new llvm::GlobalVariable(
        ir->mod,
        llvm::PointerType::getUnqual(ir->ctx),
        /*isConstant=*/false,
        llvm::GlobalVariable::ExternalLinkage,
        llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(ir->ctx)),
        "print_str");

    llvm::Constant* CPath = llvm::ConstantDataArray::getString(
        ir->ctx, "./odb-sdk/plugins/core-commands.so", /*AddNull*/ true);
    llvm::GlobalVariable* GVPath = new llvm::GlobalVariable(
        ir->mod,
        CPath->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CPath,
        "core_commands_path");
    llvm::Constant* CRTLD_LAZY
        = llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0x00001));
    llvm::Value* core_commands_lib
        = b.CreateCall(FDLOpen, {GVPath, CRTLD_LAZY});

    llvm::Constant* Sprint_str = llvm::ConstantDataArray::getString(
        ir->ctx, "print_str", /*AddNull*/ true);
    llvm::GlobalVariable* GVprint_str = new llvm::GlobalVariable(
        ir->mod,
        Sprint_str->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        Sprint_str,
        "print_str_name");
    llvm::Value* print_str_sym
        = b.CreateCall(FDLSym, {core_commands_lib, GVprint_str});
    b.CreateStore(print_str_sym, GVprint_str_func);

    switch (sdk_type)
    {
        case SDK_ODB:
            if (gen_init_odb(ir, BB, arch, platform) < 0)
                return -1;
            break;

        case SDK_DBPRO:
            if (gen_init_dbpro(ir, BB, arch, platform) < 0)
                return -1;
            break;
    }

    llvm::Function* FMainDBA = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        main_dba_name,
        ir->mod);
    b.CreateCall(FMainDBA, {});

    switch (sdk_type)
    {
        case SDK_ODB:
            if (gen_deinit_odb(ir, BB, arch, platform) < 0)
                return -1;
            break;

        case SDK_DBPRO:
            if (gen_deinit_dbpro(ir, BB, arch, platform) < 0)
                return -1;
            break;
    }

    b.CreateRet(llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0)));

    ir->mod.print(llvm::outs(), nullptr);

    return 0;
}
