#include "./ir_internal.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "odb-sdk/config.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-sdk/vec.h"
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

static int
gen_cmd_loader_odb(
    struct ir_module*         ir,
    llvm::BasicBlock*         BB,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const struct cmd_ids*     used_cmds,
    enum target_arch,
    enum target_platform platform)
{
    llvm::IRBuilder<> b(BB);

    /* Import dlopen/dlsym */
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

    /* Create a map of actually used plugin IDs */
    std::vector<bool> plugin_is_used(plugins->count);
    const cmd_id*     pcmd;
    vec_for_each(used_cmds, pcmd)
    {
        plugin_id plugin_id = cmds->plugin_ids->data[*pcmd];
        ODBSDK_DEBUG_ASSERT(
            plugin_id < plugins->count,
            log_sdk_err("plugin_id: %d\n", plugin_id));
        plugin_is_used[plugin_id] = true;
    }

    /* dlopen() all plugins and dlsym() all used command symbols */
    for (plugin_id plugin_id = 0; plugin_id != plugins->count; ++plugin_id)
    {
        if (!plugin_is_used[plugin_id])
            continue;

        struct ospath   path = plugins->data[plugin_id].filepath;
        llvm::StringRef path_ref(path.str.data, path.str.len);
        llvm::Constant* PathConstant = llvm::ConstantDataArray::getString(
            ir->ctx, path_ref, /*AddNull=*/true);
        llvm::GlobalVariable* GVPath = new llvm::GlobalVariable(
            ir->mod,
            PathConstant->getType(),
            /*isConstant=*/true,
            llvm::GlobalVariable::PrivateLinkage,
            PathConstant,
            llvm::Twine("plugin") + llvm::Twine(plugin_id) + "_path");
        llvm::Constant* CRTLD_LAZY
            = llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0x00001));
        llvm::Value* lib_handle = b.CreateCall(FDLOpen, {GVPath, CRTLD_LAZY});

        const cmd_id* pcmd;
        vec_for_each(used_cmds, pcmd)
        {
            if (cmds->plugin_ids->data[*pcmd] != plugin_id)
                continue;

            struct utf8_view c_sym_name
                = utf8_list_view(cmds->c_symbols, *pcmd);
            llvm::StringRef c_sym_name_ref(
                c_sym_name.data + c_sym_name.off, c_sym_name.len);
            llvm::Constant* SymNameConstant
                = llvm::ConstantDataArray::getString(
                    ir->ctx, c_sym_name_ref, /*AddNull=*/true);
            llvm::GlobalVariable* GVSymName = new llvm::GlobalVariable(
                ir->mod,
                SymNameConstant->getType(),
                /*isConstant=*/true,
                llvm::GlobalVariable::PrivateLinkage,
                SymNameConstant,
                llvm::Twine("cmd") + llvm::Twine(*pcmd) + "_name");

            llvm::GlobalVariable* GVCommandPtr = new llvm::GlobalVariable(
                ir->mod,
                llvm::PointerType::getUnqual(ir->ctx),
                /*isConstant=*/false,
                llvm::GlobalVariable::ExternalLinkage,
                llvm::ConstantPointerNull::get(
                    llvm::PointerType::getUnqual(ir->ctx)),
                c_sym_name_ref);

            llvm::Value* print_str_sym
                = b.CreateCall(FDLSym, {lib_handle, GVSymName});
            b.CreateStore(print_str_sym, GVCommandPtr);
        }
    }

    return 0;
}

int
ir_create_runtime(
    struct ir_module*         ir,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const struct cmd_ids*     used_cmds,
    const char*               main_dba_name,
    enum sdk_type             sdk_type,
    enum target_arch          arch,
    enum target_platform      platform)
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

    switch (sdk_type)
    {
        case SDK_ODB:
            if (gen_init_odb(ir, BB, arch, platform) != 0)
                return -1;
            if (gen_cmd_loader_odb(
                    ir, BB, plugins, cmds, used_cmds, arch, platform)
                != 0)
                return -1;
            break;

        case SDK_DBPRO:
            if (gen_init_dbpro(ir, BB, arch, platform) != 0)
                return -1;
            break;
    }

    llvm::Function* FMainDBA = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        llvm::Twine("dba_") + main_dba_name,
        ir->mod);
    b.CreateCall(FMainDBA, {});

    switch (sdk_type)
    {
        case SDK_ODB:
            if (gen_deinit_odb(ir, BB, arch, platform) != 0)
                return -1;
            break;

        case SDK_DBPRO:
            if (gen_deinit_dbpro(ir, BB, arch, platform) != 0)
                return -1;
            break;
    }

    b.CreateRet(llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0)));

    ir->mod.print(llvm::outs(), nullptr);

    return 0;
}
