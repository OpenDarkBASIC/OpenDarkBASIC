#include "./ir_internal.hpp"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/sdk_type.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-util/utf8.h"
#include "odb-util/vec.h"
}

static llvm::Value*
call_dlopen(
    struct ir_module*    ir,
    llvm::BasicBlock*    BB,
    llvm::Function*      FDLOpen,
    llvm::Value*         filepath,
    enum target_platform platform)
{
    llvm::IRBuilder<> b(BB);

    switch (platform)
    {
        case TARGET_LINUX:
        case TARGET_MACOS: {
            llvm::Constant* CRTLD_LAZY
                = llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0x00001));
            return b.CreateCall(FDLOpen, {filepath, CRTLD_LAZY});
        }

        case TARGET_WINDOWS: return b.CreateCall(FDLOpen, {filepath});
    }

    return nullptr;
}

static llvm::Value*
call_dlsym(
    struct ir_module*    ir,
    llvm::BasicBlock*    BB,
    llvm::Function*      FDLSym,
    llvm::Value*         lib_handle,
    llvm::Value*         name,
    enum target_platform platform)
{
    llvm::IRBuilder<> b(BB);
    return b.CreateCall(FDLSym, {lib_handle, name});
}

#define OFFICIAL_PLUGIN_LIST                                                   \
    X(GFX, "DBProSetupDebug")                                                  \
    X(Basic2D, "DBProBasic2DDebug")                                            \
    X(Text, "DBProTextDebug")                                                  \
    X(Transforms, "DBProTransformsDebug")                                      \
    X(Sprites, "DBProSpritesDebug")                                            \
    X(Image, "DBProImageDebug")                                                \
    X(Input, "DBProInputDebug")                                                \
    X(System, "DBProSystemDebug")                                              \
    X(Sound, "DBProSoundDebug")                                                \
    X(Music, "DBProMusicDebug")                                                \
    X(File, "DBProFileDebug")                                                  \
    X(FTP, "DBProFTPDebug")                                                    \
    X(Memblocks, "DBProMemblocksDebug")                                        \
    X(Animation, "DBProAnimationDebug")                                        \
    X(Bitmap, "DBProBitmapDebug")                                              \
    X(Multiplayer, "DBProMultiplayerDebug")                                    \
    X(Camera3D, "DBProCameraDebug")                                            \
    X(Light3D, "DBProLightDebug")                                              \
    X(Matrix3D, "DBProMatrixDebug")                                            \
    X(Basic3D, "DBProBasic3DDebug")                                            \
    X(World3D, "DBProWorld3DDebug")                                            \
    X(Q2BSP, "DBProQ2BSPDebug")                                                \
    X(OwnBSP, "DBProOwnBSPDebug")                                              \
    X(BSPCompiler, "DBProBSPCompilerDebug")                                    \
    X(Particles, "DBProParticlesDebug")                                        \
    X(PrimObject, "DBProPrimObjectDebug")                                      \
    X(Vectors, "DBProVectorsDebug")                                            \
    X(LODTerrain, "DBProLODTerrainDebug")                                      \
    X(CSG, "DBProCSGDebug")

enum official_plugin
{
#define X(name, str) OFFICIAL_PLUGIN_##name,
    OFFICIAL_PLUGIN_LIST
#undef X
        OFFICIAL_PLUGIN_COUNT
};

static const char* official_plugin_name[] = {
#define X(name, str) str,
    OFFICIAL_PLUGIN_LIST
#undef X
};

/* This list was created based on
 *   1) DBDLLCore::ConstructPostDisplayItems()
 *   2) DBDLLCore::ConstructPostDLLItems()
 *   3) Empirically by analyzing crashes using OllyDBG
 */
/* clang-format off */
static char OFFICIAL_PLUGIN_GFX_deps[] = {OFFICIAL_PLUGIN_Text, -1};
static char OFFICIAL_PLUGIN_Basic2D_deps[] = {OFFICIAL_PLUGIN_GFX, -1};
static char OFFICIAL_PLUGIN_Text_deps[] = {OFFICIAL_PLUGIN_GFX, -1};
static char OFFICIAL_PLUGIN_Image_deps[] = {OFFICIAL_PLUGIN_GFX, -1};
static char OFFICIAL_PLUGIN_Transforms_deps[] = {-1};
static char OFFICIAL_PLUGIN_Sprites_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, -1};
static char OFFICIAL_PLUGIN_Input_deps[] = {-1};
static char OFFICIAL_PLUGIN_System_deps[] = {-1};
static char OFFICIAL_PLUGIN_Sound_deps[] = {-1};
static char OFFICIAL_PLUGIN_Music_deps[] = {-1};
static char OFFICIAL_PLUGIN_File_deps[] = {-1};
static char OFFICIAL_PLUGIN_FTP_deps[] = {-1};
static char OFFICIAL_PLUGIN_Memblocks_deps[] = {-1};
static char OFFICIAL_PLUGIN_Animation_deps[] = {-1};
static char OFFICIAL_PLUGIN_Bitmap_deps[] = {-1};
static char OFFICIAL_PLUGIN_Multiplayer_deps[] = {-1};
static char OFFICIAL_PLUGIN_Camera3D_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, -1};
static char OFFICIAL_PLUGIN_Light3D_deps[] = {OFFICIAL_PLUGIN_GFX, -1};
static char OFFICIAL_PLUGIN_Matrix3D_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, -1};
static char OFFICIAL_PLUGIN_Basic3D_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, OFFICIAL_PLUGIN_Vectors, -1};
static char OFFICIAL_PLUGIN_World3D_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, OFFICIAL_PLUGIN_Camera3D, OFFICIAL_PLUGIN_Basic3D, -1};
static char OFFICIAL_PLUGIN_Q2BSP_deps[] = {-1};
static char OFFICIAL_PLUGIN_OwnBSP_deps[] = {-1};
static char OFFICIAL_PLUGIN_BSPCompiler_deps[] = {-1};
static char OFFICIAL_PLUGIN_Vectors_deps[] = {OFFICIAL_PLUGIN_GFX, -1};
static char OFFICIAL_PLUGIN_Particles_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, -1};
static char OFFICIAL_PLUGIN_LODTerrain_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, OFFICIAL_PLUGIN_Camera3D, -1};
static char OFFICIAL_PLUGIN_CSG_deps[] = {OFFICIAL_PLUGIN_GFX, -1};
static char OFFICIAL_PLUGIN_PrimObject_deps[] = {-1};
/* clang-format on */

static int
process_dependency_list(
    std::vector<bool>*       plugin_is_used,
    const std::vector<char>& official_to_plugin_id,
    official_plugin          official_plugin,
    const char*              deps)
{
    int process_more = 0;
    for (const char* dep = deps; *dep > -1; ++dep)
    {
        plugin_id plugin_id = official_to_plugin_id[*dep];
        if (plugin_id < 0)
            return log_codegen_err(
                "{quote:%s} was not found (rerquired by {quote:%s}\n",
                official_plugin_name[(int)*dep],
                official_plugin_name[official_plugin]);
        if ((*plugin_is_used)[plugin_id] == false)
        {
            (*plugin_is_used)[plugin_id] = true;
            process_more = 1;
        }
    }
    return process_more;
}

static int
enable_dependent_plugins_dbpro(
    std::vector<bool>*       plugin_is_used,
    const std::vector<char>& official_to_plugin_id)
{
    plugin_id plugin_id;
    int       process_more = 0;

#define X(name, str)                                                           \
    plugin_id = official_to_plugin_id[OFFICIAL_PLUGIN_##name];                 \
    if (plugin_id > -1 && (*plugin_is_used)[plugin_id])                        \
        switch (process_dependency_list(                                       \
            plugin_is_used,                                                    \
            official_to_plugin_id,                                             \
            OFFICIAL_PLUGIN_##name,                                            \
            OFFICIAL_PLUGIN_##name##_deps))                                    \
        {                                                                      \
            case 1: process_more = 1;                                          \
            case 0: break;                                                     \
            default: return -1;                                                \
        }
    OFFICIAL_PLUGIN_LIST
#undef X

    return process_more;
}

static int
handle_plugin_dependencies_dbpro(
    std::vector<bool>* plugin_is_used, const struct plugin_list* plugins)
{
    std::vector<char> official_to_plugin_id(OFFICIAL_PLUGIN_COUNT, -1);
    for (plugin_id plugin_id = 0; plugin_id != plugin_list_count(plugins);
         ++plugin_id)
    {
#define X(plugin_name, plugin_str)                                             \
    if (utf8_equal(                                                            \
            utf8_view(plugins->data[plugin_id].name),                          \
            cstr_utf8_view(plugin_str)))                                       \
    {                                                                          \
        official_to_plugin_id[OFFICIAL_PLUGIN_##plugin_name] = plugin_id;      \
        continue;                                                              \
    }
        OFFICIAL_PLUGIN_LIST
#undef X
    }

    /* DBProCore is always enabled */
    for (plugin_id plugin_id = 0; plugin_id != plugin_list_count(plugins);
         ++plugin_id)
        if (utf8_equal(
                utf8_view(plugins->data[plugin_id].name),
                cstr_utf8_view("DBProCore")))
        {
            (*plugin_is_used)[plugin_id] = true;
            goto core_found;
        }
    return log_codegen_err("DBProCore.dll was not found\n");
core_found:;

    /* DBProSetup is a dependency of Core */
    for (plugin_id plugin_id = 0; plugin_id != plugin_list_count(plugins);
         ++plugin_id)
        if (utf8_equal(
                utf8_view(plugins->data[plugin_id].name),
                cstr_utf8_view("DBProSetupDebug")))
        {
            (*plugin_is_used)[plugin_id] = true;
            goto text_found;
        }
    return log_codegen_err(
        "{quote:DBProSetupDebug.dll} was not found (required by "
        "{quote:DBProCore.dll})\n");
text_found:;

    /* DBProText is a dependency of Core (apparently) */
    for (plugin_id plugin_id = 0; plugin_id != plugin_list_count(plugins);
         ++plugin_id)
        if (utf8_equal(
                utf8_view(plugins->data[plugin_id].name),
                cstr_utf8_view("DBProTextDebug")))
        {
            (*plugin_is_used)[plugin_id] = true;
            goto setup_found;
        }
    return log_codegen_err(
        "{quote:DBProTextDebug.dll} was not found (required by "
        "{quote:DBProCore.dll})\n");
setup_found:;

process_more:
    switch (
        enable_dependent_plugins_dbpro(plugin_is_used, official_to_plugin_id))
    {
        case 1: goto process_more;
        case 0: break;
        default: return -1;
    }

    return 0;
}

static int
gen_cmd_loader(
    struct ir_module*              ir,
    llvm::BasicBlock*              BB,
    llvm::Function*                FDLOpen,
    llvm::Function*                FDLSym,
    llvm::StringMap<llvm::Value*>* plugin_handles,
    const struct plugin_list*      plugins,
    const struct cmd_list*         cmds,
    const struct cmd_ids*          used_cmds,
    enum sdk_type                  sdk_type,
    enum target_arch               arch,
    enum target_platform           platform)
{
    llvm::IRBuilder<> b(BB);

    /* Create a map of actually used plugin IDs */
    std::vector<bool> plugin_is_used(plugin_list_count(plugins));
    const cmd_id*     pcmd;
    vec_for_each(used_cmds, pcmd)
    {
        plugin_id plugin_id = cmds->plugin_ids->data[*pcmd];
        ODBUTIL_DEBUG_ASSERT(
            plugin_id < plugin_list_count(plugins),
            log_codegen_err("plugin_id: %d\n", plugin_id));
        plugin_is_used[plugin_id] = true;
    }

    /* Handle special cases for DBPro */
    if (sdk_type == SDK_DBPRO)
        handle_plugin_dependencies_dbpro(&plugin_is_used, plugins);

    /* dlopen() all plugins and dlsym() all used command symbols */
    for (plugin_id plugin_id = 0; plugin_id != plugin_list_count(plugins);
         ++plugin_id)
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
            llvm::Twine(".plugin") + llvm::Twine(plugin_id) + "_path");
        llvm::Value* lib_handle
            = call_dlopen(ir, BB, FDLOpen, GVPath, platform);

        struct utf8 name = plugins->data[plugin_id].name;
        plugin_handles->insert(
            {llvm::StringRef(name.data, name.len), lib_handle});

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
                llvm::Twine(".cmd") + llvm::Twine(*pcmd) + "_name");

            llvm::GlobalVariable* GVCommandPtr = new llvm::GlobalVariable(
                ir->mod,
                llvm::PointerType::getUnqual(ir->ctx),
                /*isConstant=*/false,
                llvm::GlobalVariable::ExternalLinkage,
                llvm::ConstantPointerNull::get(
                    llvm::PointerType::getUnqual(ir->ctx)),
                c_sym_name_ref);

            llvm::Value* print_str_sym
                = call_dlsym(ir, BB, FDLSym, lib_handle, GVSymName, platform);
            b.CreateStore(print_str_sym, GVCommandPtr);
        }
    }

    return 0;
}

static llvm::Function*
get_dlopen(
    struct ir_module* ir, enum target_arch arch, enum target_platform platform)
{
    switch (platform)
    {
        case TARGET_LINUX:
        case TARGET_MACOS:
            return llvm::Function::Create(
                llvm::FunctionType::get(
                    llvm::PointerType::getUnqual(ir->ctx),
                    {llvm::PointerType::getUnqual(ir->ctx),
                     llvm::Type::getInt32Ty(ir->ctx)},
                    /*isVarArg=*/false),
                llvm::Function::ExternalLinkage,
                "dlopen",
                ir->mod);

        case TARGET_WINDOWS: {
            llvm::Function* F = llvm::Function::Create(
                llvm::FunctionType::get(
                    llvm::PointerType::getUnqual(ir->ctx),
                    {llvm::PointerType::getUnqual(ir->ctx)},
                    /*isVarArg=*/false),
                llvm::Function::ExternalLinkage,
                arch == TARGET_i386 ? "LoadLibraryA@4" : "LoadLibraryA",
                ir->mod);
            F->setDLLStorageClass(llvm::GlobalValue::DLLImportStorageClass);
            return F;
        }
        break;
    }

    return nullptr;
}

static llvm::Function*
get_dlsym(
    struct ir_module* ir, enum target_arch arch, enum target_platform platform)
{
    switch (platform)
    {
        case TARGET_LINUX:
        case TARGET_MACOS:
            return llvm::Function::Create(
                llvm::FunctionType::get(
                    llvm::PointerType::getUnqual(ir->ctx),
                    {llvm::PointerType::getUnqual(ir->ctx),
                     llvm::PointerType::getUnqual(ir->ctx)},
                    /*isVarArg=*/false),
                llvm::Function::ExternalLinkage,
                "dlsym",
                ir->mod);
        case TARGET_WINDOWS:
            return llvm::Function::Create(
                llvm::FunctionType::get(
                    llvm::PointerType::getUnqual(ir->ctx),
                    {llvm::PointerType::getUnqual(ir->ctx),
                     llvm::PointerType::getUnqual(ir->ctx)},
                    /*isVarArg=*/false),
                llvm::Function::ExternalLinkage,
                arch == TARGET_i386 ? "GetProcAddress@8" : "GetProcAddress",
                ir->mod);
    }

    return nullptr;
}

int
ir_create_harness(
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

    /* Import DLL/shared lib functions */
    llvm::Function* FDLOpen = get_dlopen(ir, arch, platform);
    llvm::Function* FDLSym = get_dlsym(ir, arch, platform);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->ctx, "", F);
    llvm::IRBuilder<> b(BB);

    llvm::StringMap<llvm::Value*> plugin_handles;
    if (gen_cmd_loader(
            ir,
            BB,
            FDLOpen,
            FDLSym,
            &plugin_handles,
            plugins,
            cmds,
            used_cmds,
            sdk_type,
            arch,
            platform)
        != 0)
    {
        return -1;
    }

    llvm::Function* FSDKInit = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "odbrt_init",
        ir->mod);
    b.CreateCall(FSDKInit, {});

    llvm::Function* FMainDBA = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        llvm::Twine("dba_") + main_dba_name,
        ir->mod);
    b.CreateCall(FMainDBA, {});

    llvm::Function* FSDKDeInit = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ir->ctx), {}, false),
        llvm::Function::ExternalLinkage,
        "odbrt_exit",
        ir->mod);
    b.CreateCall(FSDKDeInit, {});

    b.CreateRet(llvm::ConstantInt::get(ir->ctx, llvm::APInt(32, 0)));

    llvm::verifyFunction(*F);

    return 0;
}
