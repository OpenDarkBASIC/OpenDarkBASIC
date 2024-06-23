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
#include "odb-sdk/utf8.h"
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
            /*AddNull=*/true);
        llvm::GlobalVariable* rpath_const = new llvm::GlobalVariable(
            ir->mod,
            rpath_data->getType(),
            /*isConstant=*/true,
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

    // Plugins use odb-sdk. Have to call the global init function
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
static char OFFICIAL_PLUGIN_Basic3D_deps[] = {OFFICIAL_PLUGIN_GFX, OFFICIAL_PLUGIN_Image, OFFICIAL_PLUGIN_Vectors -1};
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
    for (plugin_id plugin_id = 0; plugin_id != plugins->count; ++plugin_id)
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
    for (plugin_id plugin_id = 0; plugin_id != plugins->count; ++plugin_id)
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
    for (plugin_id plugin_id = 0; plugin_id != plugins->count; ++plugin_id)
        if (utf8_equal(
                utf8_view(plugins->data[plugin_id].name),
                cstr_utf8_view("DBProSetupDebug")))
        {
            (*plugin_is_used)[plugin_id] = true;
            goto setup_found;
        }
    return log_codegen_err(
        "{quote:DBProSetupDebug.dll} was not found (required by "
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
    std::vector<bool> plugin_is_used(plugins->count);
    const cmd_id*     pcmd;
    vec_for_each(used_cmds, pcmd)
    {
        plugin_id plugin_id = cmds->plugin_ids->data[*pcmd];
        ODBSDK_DEBUG_ASSERT(
            plugin_id < plugins->count,
            log_codegen_err("plugin_id: %d\n", plugin_id));
        plugin_is_used[plugin_id] = true;
    }

    /* Handle special cases for DBPro */
    if (sdk_type == SDK_DBPRO)
        handle_plugin_dependencies_dbpro(&plugin_is_used, plugins);

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

static int
gen_start_dbpro(
    struct ir_module*                    ir,
    llvm::BasicBlock*                    BB,
    llvm::Function*                      FDLOpen,
    llvm::Function*                      FDLSym,
    const llvm::StringMap<llvm::Value*>& plugins,
    enum target_arch                     arch,
    enum target_platform                 platform)
{
    llvm::IRBuilder<> b(BB);

    llvm::Function* FMemset = llvm::Intrinsic::getDeclaration(
        &ir->mod,
        llvm::Intrinsic::memset,
        {llvm::PointerType::getUnqual(ir->ctx),
         llvm::Type::getInt32Ty(ir->ctx)});
    llvm::Function* FMemcpy = llvm::Intrinsic::getDeclaration(
        &ir->mod,
        llvm::Intrinsic::memcpy,
        {llvm::PointerType::getUnqual(ir->ctx),
         llvm::PointerType::getUnqual(ir->ctx),
         llvm::Type::getInt32Ty(ir->ctx)});
    llvm::Function* FGetTempPathA = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(ir->ctx),
            {llvm::Type::getInt32Ty(ir->ctx),
             llvm::PointerType::getUnqual(ir->ctx)},
            /*isVarArg=*/false),
        llvm::Function::ExternalLinkage,
        "GetTempPathA@8",
        ir->mod);

#define GLOB_STRUCT_MEMBERS                                                    \
    /* Function Ptrs (for remote DLLs) */                                      \
    X(llvm::PointerType::getUnqual(ir->ctx), CreateDeleteString)               \
    X(llvm::PointerType::getUnqual(ir->ctx), ProcessMessageFunction)           \
    X(llvm::PointerType::getUnqual(ir->ctx), PrintStringFunction)              \
    X(llvm::PointerType::getUnqual(ir->ctx), UpdateFilenameFromVirtualTable)   \
    X(llvm::PointerType::getUnqual(ir->ctx), Decrypt)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), Encrypt)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), ChangeMouseFunction)              \
    X(llvm::PointerType::getUnqual(ir->ctx), SpareFunction1)                   \
    X(llvm::PointerType::getUnqual(ir->ctx), SpareFunction2)                   \
    X(llvm::PointerType::getUnqual(ir->ctx), SpareFunction3)                   \
                                                                               \
    /* ,Variable Memory Pt) */                                                 \
    X(llvm::PointerType::getUnqual(ir->ctx), g_pVariableSpace)                 \
                                                                               \
    /* ,Error Handler Pt) */                                                   \
    X(llvm::PointerType::getUnqual(ir->ctx), g_pErrorHandlerRef)               \
                                                                               \
    /* ,DLL Handles and Active Flag) */                                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_GFX)                            \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Text)                           \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Basic2D)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Sprites)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Image)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Input)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), g_System)                         \
    X(llvm::PointerType::getUnqual(ir->ctx), g_File)                           \
    X(llvm::PointerType::getUnqual(ir->ctx), g_FTP)                            \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Memblocks)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Bitmap)                         \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Animation)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Multiplayer)                    \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Basic3D)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Camera3D)                       \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Matrix3D)                       \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Light3D)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_World3D)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Particles)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_PrimObject)                     \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Vectors)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_XObject)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_3DSObject)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_MDLObject)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_MD2Object)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_MD3Object)                      \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Sound)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Music)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), g_LODTerrain)                     \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Q2BSP)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), g_OwnBSP)                         \
    X(llvm::PointerType::getUnqual(ir->ctx), g_BSPCompiler)                    \
    X(llvm::PointerType::getUnqual(ir->ctx), g_CSG)                            \
    X(llvm::PointerType::getUnqual(ir->ctx), g_igLoader)                       \
    X(llvm::PointerType::getUnqual(ir->ctx), g_GameFX)                         \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Transforms)                     \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare04)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare05)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare06)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare07)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare08)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare09)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare10)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare11)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare12)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare13)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare14)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare15)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare16)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare17)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare18)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare19)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), g_Spare20)                        \
    X(llvm::Type::getInt1Ty(ir->ctx), g_GFXmade)                               \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Textmade)                              \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Basic2Dmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spritesmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Imagemade)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Inputmade)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Systemmade)                            \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Filemade)                              \
    X(llvm::Type::getInt1Ty(ir->ctx), g_FTPmade)                               \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Memblocksmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Bitmapmade)                            \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Animationmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Multiplayermade)                       \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Basic3Dmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Camera3Dmade)                          \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Matrix3Dmade)                          \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Light3Dmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_World3Dmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Particlesmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_PrimObjectmade)                        \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Vectorsmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_XObjectmade)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_3DSObjectmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_MDLObjectmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_MD2Objectmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_MD3Objectmade)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Soundmade)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Musicmade)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), g_LODTerrainmade)                        \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Q2BSPmade)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), g_OwnBSPmade)                            \
    X(llvm::Type::getInt1Ty(ir->ctx), g_BSPCompilermade)                       \
    X(llvm::Type::getInt1Ty(ir->ctx), g_CSGmade)                               \
    X(llvm::Type::getInt1Ty(ir->ctx), g_igLoadermade)                          \
    X(llvm::Type::getInt1Ty(ir->ctx), g_GameFXmade)                            \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Transformsmade)                        \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare04made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare05made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare06made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare07made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare08made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare09made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare10made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare11made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare12made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare13made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare14made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare15made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare16made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare17made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare18made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare19made)                           \
    X(llvm::Type::getInt1Ty(ir->ctx), g_Spare20made)                           \
                                                                               \
    /* Executable Media Handling Data */                                       \
    X(llvm::ArrayType::get(llvm::Type::getInt8Ty(ir->ctx), 260),               \
      pEXEUnpackDirectory)                                                     \
    X(llvm::Type::getInt32Ty(ir->ctx), dwEncryptionUniqueKey)                  \
    X(llvm::Type::getInt32Ty(ir->ctx), ppEXEAbsFilename)                       \
    X(llvm::Type::getInt32Ty(ir->ctx), dwInternalFunctionCode)                 \
    X(llvm::Type::getInt32Ty(ir->ctx), g_pMachineCodeBlock)                    \
    X(llvm::Type::getInt32Ty(ir->ctx), dwEMHDSpare4)                           \
    X(llvm::Type::getInt32Ty(ir->ctx), dwEMHDSpare5)                           \
                                                                               \
    /* Windows General Data*/                                                  \
    X(llvm::PointerType::getUnqual(ir->ctx), hWnd)                             \
    X(llvm::PointerType::getUnqual(ir->ctx), hInstance)                        \
    X(llvm::PointerType::getUnqual(ir->ctx), pWindowsTextEntry)                \
    X(llvm::Type::getInt1Ty(ir->ctx), bInvalidFlag)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWindowWidth)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWindowHeight)                         \
    X(llvm::PointerType::getUnqual(ir->ctx), hAppIcon)                         \
    X(llvm::Type::getInt32Ty(ir->ctx), dwAppDisplayModeUsing)                  \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWindowX)                              \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWindowY)                              \
    X(llvm::PointerType::getUnqual(ir->ctx), hwndIGLoader)                     \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWGDSpare2)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWGDSpare3)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWGDSpare4)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWGDSpare5)                            \
                                                                               \
    /* Windows Mouse Data */                                                   \
    X(llvm::Type::getInt1Ty(ir->ctx), bWindowsMouseVisible)                    \
    X(llvm::Type::getInt32Ty(ir->ctx), iWindowsMouseX)                         \
    X(llvm::Type::getInt32Ty(ir->ctx), iWindowsMouseY)                         \
    X(llvm::Type::getInt32Ty(ir->ctx), iWindowsMouseClick)                     \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWindowsMouseLeftTouchPersist)         \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWMDSpare3)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWMDSpare4)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwWMDSpare5)                            \
                                                                               \
    /* Main Screen Data (backbuffer)*/                                         \
    X(llvm::Type::getInt32Ty(ir->ctx), iScreenWidth)                           \
    X(llvm::Type::getInt32Ty(ir->ctx), iScreenHeight)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), iScreenDepth)                           \
    X(llvm::Type::getInt32Ty(ir->ctx), iNoDrawLeft)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), iNoDrawTop)                             \
    X(llvm::Type::getInt32Ty(ir->ctx), iNoDrawRight)                           \
    X(llvm::Type::getInt32Ty(ir->ctx), iNoDrawBottom)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), dwSafeRectMax)                          \
    X(llvm::PointerType::getUnqual(ir->ctx), pSafeRects)                       \
    X(llvm::Type::getInt32Ty(ir->ctx), dwMSDSpare3)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwMSDSpare4)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwMSDSpare5)                            \
                                                                               \
    /* Bitmap and Surface Data (for drawing offscreen) */                      \
    X(llvm::Type::getInt32Ty(ir->ctx), iCurrentBitmapNumber)                   \
    X(llvm::PointerType::getUnqual(ir->ctx), pCurrentBitmapTexture)            \
    X(llvm::PointerType::getUnqual(ir->ctx), pCurrentBitmapSurface)            \
    X(llvm::PointerType::getUnqual(ir->ctx), pHoldBackBufferPtr)               \
    X(llvm::PointerType::getUnqual(ir->ctx), pHoldDepthBufferPtr)              \
    X(llvm::Type::getInt32Ty(ir->ctx), dwBSDSpare1)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwBSDSpare2)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwBSDSpare3)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwBSDSpare4)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwBSDSpare5)                            \
                                                                               \
    /* Drawing Data */                                                         \
    X(llvm::Type::getInt32Ty(ir->ctx), iCursorX)                               \
    X(llvm::Type::getInt32Ty(ir->ctx), iCursorY)                               \
    X(llvm::Type::getInt32Ty(ir->ctx), dwForeColor)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwBackColor)                            \
    X(llvm::Type::getInt32Ty(ir->ctx), dwRenderCameraID)                       \
    X(llvm::Type::getFloatTy(ir->ctx), fReflectionPlaneX)                      \
    X(llvm::Type::getFloatTy(ir->ctx), fReflectionPlaneY)                      \
    X(llvm::Type::getFloatTy(ir->ctx), fReflectionPlaneZ)                      \
    X(llvm::Type::getInt32Ty(ir->ctx), dwCurrentSetCameraID)                   \
    X(llvm::PointerType::getUnqual(ir->ctx), lpDirectXVersionString)           \
    X(llvm::Type::getInt32Ty(ir->ctx), dw3DBackColor)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), dwDDSpare4)                             \
    X(llvm::Type::getInt32Ty(ir->ctx), dwDDSpare5)                             \
                                                                               \
    /* Checklist Data */                                                       \
    X(llvm::Type::getInt1Ty(ir->ctx), checklistexists)                         \
    X(llvm::Type::getInt1Ty(ir->ctx), checklisthasvalues)                      \
    X(llvm::Type::getInt1Ty(ir->ctx), checklisthasstrings)                     \
    X(llvm::Type::getInt32Ty(ir->ctx), checklistqty)                           \
    X(llvm::Type::getInt32Ty(ir->ctx), dwChecklistArraySize)                   \
    X(llvm::PointerType::getUnqual(ir->ctx), checklist)                        \
                                                                               \
    /* Dependent 3D Data Exchange */                                           \
    X(llvm::Type::getInt32Ty(ir->ctx), iFogState)                              \
    X(llvm::Type::getInt32Ty(ir->ctx), dwRedrawPhase)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), dwRedrawCount)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), dwStencilMode)                          \
    X(llvm::Type::getInt32Ty(ir->ctx), dwStencilShadowCount)                   \
    X(llvm::Type::getInt32Ty(ir->ctx), dwStencilReflectionCount)               \
    X(llvm::Type::getInt32Ty(ir->ctx), dwNumberOfPolygonsDrawn)                \
    X(llvm::Type::getInt32Ty(ir->ctx), dwNumberOfPrimCalls)                    \
    X(llvm::Type::getInt32Ty(ir->ctx), dwStencilSpare3)                        \
    X(llvm::Type::getInt32Ty(ir->ctx), dwStencilSpare4)                        \
    X(llvm::Type::getInt32Ty(ir->ctx), dwStencilSpare5)                        \
                                                                               \
    /* System States and Global Controls */                                    \
    X(llvm::Type::getInt1Ty(ir->ctx), bEscapeKeyEnabled)                       \
    X(llvm::Type::getInt1Ty(ir->ctx), bSystemKeyEnabled)                       \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool1)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool2)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool3)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool4)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool5)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool6)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool7)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool8)                             \
    X(llvm::Type::getInt1Ty(ir->ctx), bSpareBool9)                             \
    X(llvm::PointerType::getUnqual(ir->ctx), pExitPromptString)                \
    X(llvm::PointerType::getUnqual(ir->ctx), pExitPromptString2)               \
    X(llvm::Type::getInt32Ty(ir->ctx), iSoftwareVP)                            \
                                                                               \
    /* Dynamic Memory Area for future expansion */                             \
    X(llvm::Type::getInt32Ty(ir->ctx), dwDynMemSize)                           \
    X(llvm::PointerType::getUnqual(ir->ctx), pDynMemPtr)
    enum GlobStructMember
    {
#define X(Ty, name) name,
        GLOB_STRUCT_MEMBERS
#undef X
    };

    llvm::StructType* TGlobStruct = llvm::StructType::get(
        ir->ctx,
        {
#define X(Ty, name) Ty,
            GLOB_STRUCT_MEMBERS
#undef X
        },
        /*isPacked=*/false);

    /* Load GetGlobPtr() function from DBProCore DLL */
    llvm::Constant* CGetGlobPtr = llvm::ConstantDataArray::getString(
        ir->ctx, "?GetGlobPtr@@YAKXZ", /*AddNull=*/true);
    llvm::GlobalVariable* GVGetGlobPtr = new llvm::GlobalVariable(
        ir->mod,
        CGetGlobPtr->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CGetGlobPtr,
        ".CORE_GetGlobPtr_name");
    llvm::Value* GetGlobPtr = call_dlsym(
        ir,
        BB,
        FDLSym,
        plugins.find("DBProCore")->getValue(),
        GVGetGlobPtr,
        platform);

    /* Call GetGlobPtr(). Returns a DWORD, but has to be cast to a pointer type
     */
    llvm::FunctionType* FTGetGlobPtr = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(ir->ctx), {}, /*isVarArg=*/false);
    llvm::Value* glob_ptr_as_dword = b.CreateCall(FTGetGlobPtr, GetGlobPtr, {});
    llvm::Value* glob_ptr = b.CreateIntToPtr(
        glob_ptr_as_dword, llvm::PointerType::getUnqual(ir->ctx));

    /* Fill in the "official" plugin handles into the glob struct wherever
     * applicable */
#define X(name, str)                                                           \
    {                                                                          \
        auto it = plugins.find(str);                                           \
        if (it != plugins.end())                                               \
        {                                                                      \
            llvm::Value* lib_handle = it->getValue();                          \
            llvm::Value* Member                                                \
                = b.CreateStructGEP(TGlobStruct, glob_ptr, g_##name);          \
            b.CreateStore(lib_handle, Member);                                 \
        }                                                                      \
    }
    // OFFICIAL_PLUGIN_LIST
#undef X

    /* Set pEXEUnpackDirectory to a temporary path using GetTempPathA()
     * Code equivalent:
     *   memset(globPtr->pEXEUnpackDirectory, 0,
     *       sizeof(globPtr->pEXEUnpackDirectory));
     *   int tempPathSize = GetTempPathA(MAX_PATH,
     *       globPtr->pEXEUnpackDirectory);
     *   char unpackDir[] = "odbc-unpack";
     *   memcpy(globPtr->pEXEUnpackDirectory + tempPathSize,
     *       unpackDir, sizeof(unpackDir));
     */
    llvm::Value* pEXEUnpackDirectoryPtr = b.CreateGEP(
        TGlobStruct,
        glob_ptr,
        {b.getInt32(0), b.getInt32(pEXEUnpackDirectory), b.getInt32(0)});
    b.CreateCall(
        FMemset,
        {pEXEUnpackDirectoryPtr,
         b.getInt8(0),
         b.getInt32(260),
         /*isVolatile=*/b.getInt1(false)});
    llvm::Value* tempPathSize = b.CreateCall(
        FGetTempPathA, {b.getInt32(260), pEXEUnpackDirectoryPtr});
    llvm::Constant* CODBUnpackDir = llvm::ConstantDataArray::getString(
        ir->ctx,
        "odb-unpack",
        /*AddNull=*/true);
    llvm::GlobalVariable* GVODBUnpackDir = new llvm::GlobalVariable(
        ir->mod,
        CODBUnpackDir->getType(),
        /*isConstant*/ true,
        llvm::GlobalValue::PrivateLinkage,
        CODBUnpackDir,
        ".ODBUnpackDir");
    llvm::Value* AppendPathPtr = b.CreateGEP(
        llvm::Type::getInt8Ty(ir->ctx), pEXEUnpackDirectoryPtr, tempPathSize);
    b.CreateCall(
        FMemcpy,
        {AppendPathPtr,
         GVODBUnpackDir,
         b.getInt32(sizeof("odb-unpack")),
         /*isVolatile=*/b.getInt1(false)});

    /* DBP stores a global "errno"-like variable that contains the last error,
     * if any. The memory for this variable is managed externally, so we have to
     * pass it in. It is a DWORD */
    llvm::GlobalVariable* GVLastError = new llvm::GlobalVariable(
        ir->mod,
        llvm::Type::getInt32Ty(ir->ctx),
        /*isConstant=*/false,
        llvm::GlobalVariable::InternalLinkage,
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ir->ctx), 0),
        "LastDBPError");
    llvm::Constant* CPassErrorHandlerPtr = llvm::ConstantDataArray::getString(
        ir->ctx, "?PassErrorHandlerPtr@@YAXPAX@Z", /*AddNull=*/true);
    llvm::GlobalVariable* GVPassErrorHandlerPtr = new llvm::GlobalVariable(
        ir->mod,
        CPassErrorHandlerPtr->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CPassErrorHandlerPtr,
        ".CORE_PassErrorHandlerPtr_name");
    llvm::Value* PassErrorHandlerPtr = call_dlsym(
        ir,
        BB,
        FDLSym,
        plugins.find("DBProCore")->getValue(),
        GVPassErrorHandlerPtr,
        platform);
    llvm::FunctionType* FTPassErrorHandlerPtr = llvm::FunctionType::get(
        llvm::Type::getVoidTy(ir->ctx),
        {llvm::PointerType::getUnqual(ir->ctx)},
        /*isVarArg=*/false);
    b.CreateCall(FTPassErrorHandlerPtr, PassErrorHandlerPtr, {GVLastError});

    /* PassDLLs */
    llvm::Constant* CPassDLLs = llvm::ConstantDataArray::getString(
        ir->ctx, "?PassDLLs@@YAXXZ", /*AddNull=*/true);
    llvm::GlobalVariable* GVPassDLLs = new llvm::GlobalVariable(
        ir->mod,
        CPassDLLs->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CPassDLLs,
        ".CORE_PassDLLs_name");
    llvm::Value* PassDLLs = call_dlsym(
        ir,
        BB,
        FDLSym,
        plugins.find("DBProCore")->getValue(),
        GVPassDLLs,
        platform);
    llvm::FunctionType* FTPassDLLs = llvm::FunctionType::get(
        llvm::Type::getVoidTy(ir->ctx),
        {},
        /*isVarArg=*/false);
    b.CreateCall(FTPassDLLs, PassDLLs, {});

    /* InitDisplay */
    llvm::Constant* CInitDisplay = llvm::ConstantDataArray::getString(
        ir->ctx,
        "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z",
        /*AddNull=*/true);
    llvm::GlobalVariable* GVInitDisplay = new llvm::GlobalVariable(
        ir->mod,
        CInitDisplay->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CInitDisplay,
        ".CORE_InitDisplay_name");
    llvm::Value* InitDisplay = call_dlsym(
        ir,
        BB,
        FDLSym,
        plugins.find("DBProCore")->getValue(),
        GVInitDisplay,
        platform);
    llvm::FunctionType* FTInitDisplay = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(ir->ctx),
        {llvm::Type::getInt32Ty(ir->ctx),
         llvm::Type::getInt32Ty(ir->ctx),
         llvm::Type::getInt32Ty(ir->ctx),
         llvm::Type::getInt32Ty(ir->ctx),
         llvm::PointerType::getUnqual(ir->ctx),
         llvm::PointerType::getUnqual(ir->ctx)},
        /*isVarArg=*/false);
    b.CreateCall(
        FTInitDisplay,
        InitDisplay,
        {b.getInt32(1),
         b.getInt32(640),
         b.getInt32(480),
         b.getInt32(32),
         llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(ir->ctx)),
         llvm::ConstantPointerNull::get(
             llvm::PointerType::getUnqual(ir->ctx))});

    /* Pass core data pointer to each plugin */
    llvm::Constant* CReceiveCoreDataPtrCXX = llvm::ConstantDataArray::getString(
        ir->ctx,
        "?ReceiveCoreDataPtr@@YAXPAX@Z",
        /*AddNull=*/true);
    llvm::Constant* CReceiveCoreDataPtrC = llvm::ConstantDataArray::getString(
        ir->ctx,
        "ReceiveCoreDataPtr",
        /*AddNull=*/true);
    llvm::GlobalVariable* GVReceiveCoreDataPtrCXX = new llvm::GlobalVariable(
        ir->mod,
        CReceiveCoreDataPtrCXX->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CReceiveCoreDataPtrCXX,
        ".ReceiveCoreDataPtrCXX_name");
    llvm::GlobalVariable* GVReceiveCoreDataPtrC = new llvm::GlobalVariable(
        ir->mod,
        CReceiveCoreDataPtrC->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CReceiveCoreDataPtrC,
        ".ReceiveCoreDataPtrC_name");
#define X(name, str)                                                           \
    {                                                                          \
        auto it = plugins.find(str);                                           \
        if (it != plugins.end())                                               \
        {                                                                      \
            llvm::Value* lib_handle = it->getValue();                          \
            llvm::Value* ReceiveCoreDataPtrCXX = call_dlsym(                   \
                ir,                                                            \
                BB,                                                            \
                FDLSym,                                                        \
                lib_handle,                                                    \
                GVReceiveCoreDataPtrCXX,                                       \
                platform);                                                     \
        }                                                                      \
    }
    OFFICIAL_PLUGIN_LIST
    /* TODO */
#undef X

    /* Tells DBProCore to call each official plugin's Construct() function */
    llvm::Constant* CConstructDLLs = llvm::ConstantDataArray::getString(
        ir->ctx, "?ConstructDLLs@@YAXXZ", /*AddNull=*/true);
    llvm::GlobalVariable* GVConstructDLLs = new llvm::GlobalVariable(
        ir->mod,
        CConstructDLLs->getType(),
        /*isConstant=*/true,
        llvm::GlobalVariable::PrivateLinkage,
        CConstructDLLs,
        ".CORE_ConstructDLLs_name");
    llvm::Value* ConstructDLLs = call_dlsym(
        ir,
        BB,
        FDLSym,
        plugins.find("DBProCore")->getValue(),
        GVConstructDLLs,
        platform);
    llvm::FunctionType* FTConstructDLLs = llvm::FunctionType::get(
        llvm::Type::getVoidTy(ir->ctx),
        {},
        /*isVarArg=*/false);
    b.CreateCall(FTConstructDLLs, ConstructDLLs, {});

    return 0;
}

static llvm::Function*
get_dlopen(struct ir_module* ir, enum target_platform platform)
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
                "LoadLibraryA@4",
                ir->mod);
            // F->setDLLStorageClass(llvm::GlobalValue::DLLImportStorageClass);
            return F;
        }
        break;
    }

    return nullptr;
}

static llvm::Function*
get_dlsym(struct ir_module* ir, enum target_platform platform)
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
                "GetProcAddress@8",
                ir->mod);
    }

    return nullptr;
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

    /* Import DLL/shared lib functions */
    llvm::Function* FDLOpen = get_dlopen(ir, platform);
    llvm::Function* FDLSym = get_dlsym(ir, platform);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ir->ctx, "", F);
    llvm::IRBuilder<> b(BB);

    llvm::StringMap<llvm::Value*> plugin_handles;
    switch (sdk_type)
    {
        case SDK_ODB:
            if (gen_init_odb(ir, BB, arch, platform) != 0)
                return -1;
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
            break;

        case SDK_DBPRO:
            if (gen_init_dbpro(ir, BB, arch, platform) != 0)
                return -1;
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
            if (gen_start_dbpro(
                    ir, BB, FDLOpen, FDLSym, plugin_handles, arch, platform)
                != 0)
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

    llvm::verifyFunction(*F);

    return 0;
}
