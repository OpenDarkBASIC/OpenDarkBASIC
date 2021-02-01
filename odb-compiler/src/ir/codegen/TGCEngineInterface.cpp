#include "TGCEngineInterface.hpp"
#include "odb-sdk/DynamicLibrary.hpp"

#include <filesystem>
#include <iostream>

namespace odb::ir {
namespace {
std::string getPluginName(const DynamicLibrary* library)
{
    return std::filesystem::path{library->getFilename()}.stem().string();
}

} // namespace
TGCEngineInterface::TGCEngineInterface(llvm::Module& module) : EngineInterface(module)
{
    // Declare required WinAPI functions.
    /*
         using FuncPtr = int (__stdcall *)();
        __declspec(dllimport) void* __stdcall LoadLibraryA(const char* lpLibFileName);
        __declspec(dllimport) FuncPtr GetProcAddress(void* hModule, const char* lpProcName);
     */

    hInstanceTy = llvm::Type::getInt8PtrTy(ctx);
    stringTy = llvm::Type::getInt8PtrTy(ctx);
    procAddrTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), false)->getPointerTo();
    dwordTy = llvm::Type::getInt32Ty(ctx);

    // Declare LoadLibraryA from Kernel32.lib
    loadLibraryFunc = llvm::Function::Create(llvm::FunctionType::get(hInstanceTy, {stringTy}, false),
                                             llvm::Function::ExternalLinkage, "LoadLibraryA", module);
    loadLibraryFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    loadLibraryFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetLastError from Kernel32.lib
    getLastErrorFunc = llvm::Function::Create(llvm::FunctionType::get(dwordTy, {}, false),
                                              llvm::Function::ExternalLinkage, "GetLastError", module);
    getLastErrorFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    getLastErrorFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetProcAddress from Kernel32.lib
    getProcAddrFunc = llvm::Function::Create(llvm::FunctionType::get(procAddrTy, {hInstanceTy, stringTy}, false),
                                             llvm::Function::ExternalLinkage, "GetProcAddress", module);
    getProcAddrFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    getProcAddrFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetModuleHandle from Kernel32.lib
    getModuleHandleFunc = llvm::Function::Create(llvm::FunctionType::get(hInstanceTy, {stringTy}, false),
                                                 llvm::Function::ExternalLinkage, "GetModuleHandleA", module);
    getModuleHandleFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    getModuleHandleFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Define GlobStruct
    /*
        struct GlobStruct {
            char Padding[48];
        0   HINSTANCE GFX;
        1   HINSTANCE Text;
        2   HINSTANCE Basic2D;
        3   HINSTANCE Sprites;
        4   HINSTANCE Image;
        5   HINSTANCE Input;
        6   HINSTANCE System;
        7   HINSTANCE File;
        8   HINSTANCE FTP;
        9   HINSTANCE Memblocks;
        10  HINSTANCE Bitmap;
        11  HINSTANCE Animation;
        12  HINSTANCE Multiplayer;
        13  HINSTANCE Basic3D;
        14  HINSTANCE Camera3D;
        15  HINSTANCE Matrix3D;
        16  HINSTANCE Light3D;
        17  HINSTANCE World3D;
        18  HINSTANCE Particles;
        19  HINSTANCE PrimObject;
        20  HINSTANCE Vectors;
        21  HINSTANCE XObject;
        22  HINSTANCE 3DSObject;
        23  HINSTANCE MDLObject;
        24  HINSTANCE MD2Object;
        25  HINSTANCE MD3Object;
        26  HINSTANCE Sound;
        27  HINSTANCE Music;
        28  HINSTANCE LODTerrain;
        29  HINSTANCE Q2BSP;
        30  HINSTANCE OwnBSP;
        31  HINSTANCE BSPCompiler;
        32  HINSTANCE CSG;
        33  HINSTANCE igLoader;
        34  HINSTANCE GameFX;
        35  HINSTANCE Transforms;
            char padding2[49];
            char pEXEUnpackDirectory[_MAX_PATH]; // _MAX_PATH = 260
            DWORD dwEncryptionUniqueKey;
            DWORD ppEXEAbsFilename;
            // Extra parts of GlobStruct that we don't care about has been omitted
        };
    */
    std::vector<llvm::Type*> globStructElements;
    // padding.
    globStructElements.emplace_back(llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 48));
    // 36 HINSTANCE's for each plugin.
    globStructElements.emplace_back(llvm::ArrayType::get(hInstanceTy, 36));
    // padding.
    globStructElements.emplace_back(llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 30));
    globStructTy = llvm::StructType::get(ctx, globStructElements);

    // GlobStruct consists of 36 HINSTANCE's which we need to associate with different plugins.
    // Indexes are taken from the block comment above.
    pluginGlobStructIndices["DBProSetupDebug"] = 0;
    pluginGlobStructIndices["DBProBasic2DDebug"] = 2;
    pluginGlobStructIndices["DBProTextDebug"] = 1;
    pluginGlobStructIndices["DBProTransformsDebug"] = 35;
    pluginGlobStructIndices["DBProSpritesDebug"] = 3;
    pluginGlobStructIndices["DBProImageDebug"] = 4;
    pluginGlobStructIndices["DBProInputDebug"] = 5;
    pluginGlobStructIndices["DBProSystemDebug"] = 6;
    pluginGlobStructIndices["DBProSoundDebug"] = 26;
    pluginGlobStructIndices["DBProMusicDebug"] = 27;
    pluginGlobStructIndices["DBProFileDebug"] = 7;
    pluginGlobStructIndices["DBProFTPDebug"] = 8;
    pluginGlobStructIndices["DBProMemblocksDebug"] = 9;
    pluginGlobStructIndices["DBProAnimationDebug"] = 11;
    pluginGlobStructIndices["DBProBitmapDebug"] = 10;
    pluginGlobStructIndices["DBProMultiplayerDebug"] = 12;
    pluginGlobStructIndices["DBProCameraDebug"] = 14;
    pluginGlobStructIndices["DBProLightDebug"] = 16;
    pluginGlobStructIndices["DBProMatrixDebug"] = 15;
    pluginGlobStructIndices["DBProBasic3DDebug"] = 13;
    pluginGlobStructIndices["DBProWorld3DDebug"] = 17;
    pluginGlobStructIndices["DBProQ2BSPDebug"] = 29;
    pluginGlobStructIndices["DBProOwnBSPDebug"] = 30;
    pluginGlobStructIndices["DBProBSPCompilerDebug"] = 31;
    pluginGlobStructIndices["DBProParticlesDebug"] = 18;
    pluginGlobStructIndices["DBProPrimObjectDebug"] = 19;
    pluginGlobStructIndices["DBProVectorsDebug"] = 20;
    pluginGlobStructIndices["DBProLODTerrainDebug"] = 28;
    pluginGlobStructIndices["DBProCSGDebug"] = 32;
}

llvm::Function* TGCEngineInterface::generateCommandCall(const cmd::Command& command, const std::string& functionName,
                                                        llvm::FunctionType* functionType)
{
    llvm::Function* function =
        llvm::Function::Create(functionType, llvm::Function::InternalLinkage, functionName, module);

    llvm::IRBuilder<> builder(module.getContext());
    llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(module.getContext(), "", function);
    builder.SetInsertPoint(basicBlock);

    llvm::Type* pluginReturnType = functionType->getReturnType();
    if (functionType->getReturnType()->isFloatTy())
    {
        pluginReturnType = dwordTy;
    }
    llvm::FunctionType* pluginFunctionType =
        llvm::FunctionType::get(pluginReturnType, functionType->params(), functionType->isVarArg());

    // Obtain function ptr from the relevant plugin.
    // TODO: Call this once at the beginning of the application.
    auto commandFunction =
        getPluginFunction(builder, pluginFunctionType, command.library(), command.cppSymbol(), functionName + "Symbol");

    //    printString(builder, builder.CreateGlobalStringPtr("Calling " + functionName));

    // Call it.
    std::vector<llvm::Value*> forwardedArgs;
    for (llvm::Argument& arg : function->args())
    {
        forwardedArgs.emplace_back(&arg);
    }
    llvm::CallInst* commandResult = builder.CreateCall(commandFunction, forwardedArgs);
    //    printString(builder, builder.CreateGlobalStringPtr("Finished calling " + functionName));
    if (functionType->getReturnType()->isVoidTy())
    {
        builder.CreateRetVoid();
    }
    else if (functionType->getReturnType()->isFloatTy())
    {
        llvm::Value* dwordStoragePtr = builder.CreateAlloca(dwordTy);
        builder.CreateStore(commandResult, dwordStoragePtr);
        llvm::Value* dwordAsFloatStorage = builder.CreateBitCast(dwordStoragePtr, llvm::Type::getFloatPtrTy(ctx));
        builder.CreateRet(builder.CreateLoad(dwordAsFloatStorage));
    }
    else
    {
        builder.CreateRet(commandResult);
    }
    return function;
}

void TGCEngineInterface::generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibrary*> pluginsToLoad)
{
    if (pluginsToLoad.empty())
    {
        // TODO: Fatal error.
        fprintf(stderr, "FATAL ERROR: No plugins specified.\n");
        std::terminate();
    }

    // Ensuring that DBProCore.dll is the first plugin.
    auto isCorePlugin = [](const DynamicLibrary* library) -> bool
    {
        return std::filesystem::path{library->getFilename()}.stem() == "DBProCore";
    };
    for (int i = 0; i < pluginsToLoad.size(); ++i)
    {
        if (isCorePlugin(pluginsToLoad[i]))
        {
            // Swap with front.
            std::swap(pluginsToLoad[0], pluginsToLoad[i]);
            break;
        }
    }
    if (!isCorePlugin(pluginsToLoad[0]))
    {
        // TODO: Fatal error.
        fprintf(stderr, "FATAL ERROR: DBProCore.dll is missing.\n");
        std::terminate();
    }

    DynamicLibrary* corePlugin = pluginsToLoad[0];

    // Remove plugins that we haven't used.
    //    pluginsToLoad.erase(std::remove_if(pluginsToLoad.begin(), pluginsToLoad.end(),
    //    [this](const std::string& plugin) {
    //        return pluginHModulePtrs.count(plugin) == 0;
    //    }), pluginsToLoad.end());

    // Create main function.
    llvm::Function* entryPointFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}),
                                                            llvm::Function::ExternalLinkage, "main", module);
    llvm::IRBuilder<> builder(ctx);

    // Initialisation blocks.
    std::vector<llvm::BasicBlock*> pluginLoadingBlocks;
    pluginLoadingBlocks.reserve(pluginsToLoad.size());
    for (const auto& plugin : pluginsToLoad)
    {
        pluginLoadingBlocks.emplace_back(llvm::BasicBlock::Create(ctx, "load" + getPluginName(plugin), entryPointFunc));
    }
    llvm::BasicBlock* failedToLoadPluginsBlock = llvm::BasicBlock::Create(ctx, "failedToLoadPlugin", entryPointFunc);
    llvm::BasicBlock* initialiseEngineBlock = llvm::BasicBlock::Create(ctx, "initialiseEngine", entryPointFunc);
    for (int i = 0; i < pluginsToLoad.size(); ++i)
    {
        DynamicLibrary* plugin = pluginsToLoad[i];
        std::string pluginName = getPluginName(plugin);
        std::string pluginPath = std::filesystem::path{plugin->getFilename()}.filename().string();

        builder.SetInsertPoint(pluginLoadingBlocks[i]);

        // Call LoadLibrary and store handle.
        auto* pluginNameConstant = builder.CreateGlobalStringPtr(pluginPath);
        auto* libraryHModule = builder.CreateCall(
            loadLibraryFunc, {builder.CreateBitCast(pluginNameConstant, llvm::Type::getInt8PtrTy(ctx))});
        libraryHModule->setCallingConv(llvm::CallingConv::X86_StdCall);
        builder.CreateStore(libraryHModule, getOrAddPluginHModule(plugin));

        // Print that we've trying to load that plugin.
        printString(builder, builder.CreateGlobalStringPtr("Loading plugin " + pluginName));

        // Check if loaded successfully.
        auto* nextBlock = i == (pluginsToLoad.size() - 1) ? initialiseEngineBlock : pluginLoadingBlocks[i + 1];
        builder.CreateCondBr(builder.CreateICmpNE(libraryHModule, llvm::ConstantPointerNull::get(hInstanceTy)),
                             nextBlock, failedToLoadPluginsBlock);
    }

    // Handle plugin failure.
    builder.SetInsertPoint(failedToLoadPluginsBlock);
    printString(builder, builder.CreateGlobalStringPtr("Failed to load plugin. GetLastError returned"));
    auto* getLastErrorCall = builder.CreateCall(getLastErrorFunc, {});
    getLastErrorCall->setCallingConv(llvm::CallingConv::X86_StdCall);
    printString(builder, convertIntegerToString(builder, getLastErrorCall));
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));

    // Set plugin handles in GlobStruct.
    builder.SetInsertPoint(initialiseEngineBlock);
    printString(builder, builder.CreateGlobalStringPtr("Initialising engine."));
    auto getGlobStructFunc =
        getPluginFunction(builder, llvm::FunctionType::get(dwordTy, false), corePlugin, "?GetGlobPtr@@YAKXZ");
    llvm::Value* globStructPtr =
        builder.CreateIntToPtr(builder.CreateCall(getGlobStructFunc), globStructTy->getPointerTo());
    for (const auto& [plugin, globStructIndex] : pluginGlobStructIndices)
    {
        auto pluginHModuleEntry = pluginHModulePtrs.find(plugin);
        // Index the array in GlobStruct.
        std::vector<llvm::Value*> gepIndices;
        gepIndices.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        gepIndices.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));
        gepIndices.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), globStructIndex));
        llvm::Value* hModuleElement = builder.CreateGEP(globStructPtr, gepIndices);
        if (pluginHModuleEntry == pluginHModulePtrs.end())
        {
            builder.CreateStore(llvm::ConstantPointerNull::get(hInstanceTy), hModuleElement);
        }
        else
        {
            builder.CreateStore(builder.CreateLoad(pluginHModuleEntry->second), hModuleElement);
        }
    }

    // Initialise engine.
    auto passErrorPtrFunc = getPluginFunction(
        builder, llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {dwordTy->getPointerTo()}, false), corePlugin,
        "?PassErrorHandlerPtr@@YAXPAX@Z");
    auto passDLLsFunc = getPluginFunction(builder, llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), false),
                                          corePlugin, "?PassDLLs@@YAXXZ");
    auto initDisplayFunc = getPluginFunction(
        builder, llvm::FunctionType::get(dwordTy, {dwordTy, dwordTy, dwordTy, dwordTy, hInstanceTy, stringTy}, false),
        corePlugin, "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z");
    auto closeDisplayFunc =
        getPluginFunction(builder, llvm::FunctionType::get(dwordTy, false), corePlugin, "?CloseDisplay@@YAKXZ");

    llvm::Value* runtimeErrorPtr = builder.CreateAlloca(dwordTy);
    builder.CreateStore(llvm::ConstantInt::get(dwordTy, 0), runtimeErrorPtr);
    builder.CreateCall(passErrorPtrFunc, {runtimeErrorPtr});
    builder.CreateCall(passDLLsFunc);

    int initialDisplayMode = 1;
    int initialDisplayWidth = 640;
    int initialDisplayHeight = 480;
    int initialDisplayDepth = 32;
    llvm::Value* initDisplayResult = builder.CreateCall(
        initDisplayFunc,
        {llvm::ConstantInt::get(dwordTy, initialDisplayMode), llvm::ConstantInt::get(dwordTy, initialDisplayWidth),
         llvm::ConstantInt::get(dwordTy, initialDisplayHeight), llvm::ConstantInt::get(dwordTy, initialDisplayDepth),
         builder.CreateCall(getModuleHandleFunc, {llvm::ConstantPointerNull::get(stringTy)}),
         llvm::ConstantPointerNull::get(stringTy)});

    llvm::BasicBlock* failedToInitDisplayBlock = llvm::BasicBlock::Create(ctx, "failedToInitDisplay", entryPointFunc);
    llvm::BasicBlock* launchGameBlock = llvm::BasicBlock::Create(ctx, "launchGame", entryPointFunc);

    builder.CreateCondBr(builder.CreateICmpEQ(initDisplayResult, llvm::ConstantInt::get(dwordTy, 0)), launchGameBlock,
                         failedToInitDisplayBlock);

    builder.SetInsertPoint(failedToInitDisplayBlock);
    printString(builder, builder.CreateGlobalStringPtr("Failed to init display."));
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));

    // Launch application and exit.
    builder.SetInsertPoint(launchGameBlock);
    printString(builder, builder.CreateGlobalStringPtr("Running game."));
    builder.CreateCall(gameEntryPoint, {});
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
}

llvm::Value* TGCEngineInterface::getOrAddPluginHModule(const DynamicLibrary* library)
{
    auto pluginName = getPluginName(library);

    auto pluginHModuleIt = pluginHModulePtrs.find(pluginName);
    if (pluginHModuleIt != pluginHModulePtrs.end())
    {
        return pluginHModuleIt->second;
    }

    auto* pluginHModule = new llvm::GlobalVariable(module, hInstanceTy, false, llvm::GlobalValue::InternalLinkage,
                                                   llvm::ConstantPointerNull::get(hInstanceTy), pluginName + "HModule");
    pluginHModulePtrs.emplace(pluginName, pluginHModule);
    return pluginHModule;
}

llvm::FunctionCallee TGCEngineInterface::getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                                           const DynamicLibrary* library, const std::string& symbol,
                                                           const std::string& symbolStringName)
{
    llvm::Value* pluginHModule = builder.CreateLoad(getOrAddPluginHModule(library));
    llvm::CallInst* procAddress =
        builder.CreateCall(getProcAddrFunc, {pluginHModule, builder.CreateGlobalStringPtr(symbol, symbolStringName)});
    procAddress->setCallingConv(llvm::CallingConv::X86_StdCall);
    return llvm::FunctionCallee(functionTy, builder.CreateBitCast(procAddress, functionTy->getPointerTo()));
}

void TGCEngineInterface::printString(llvm::IRBuilder<>& builder, llvm::Value* string)
{
    llvm::Function* putsFunc = module.getFunction("puts");
    if (!putsFunc)
    {
        putsFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {stringTy}, false),
                                          llvm::Function::ExternalLinkage, "puts", module);
        putsFunc->setCallingConv(llvm::CallingConv::C);
        putsFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    }
    builder.CreateCall(putsFunc, {string});
}

llvm::Value* TGCEngineInterface::convertIntegerToString(llvm::IRBuilder<>& builder, llvm::Value* integer)
{
    llvm::Function* ltoaFunc = module.getFunction("_itoa");
    if (!ltoaFunc)
    {
        ltoaFunc = llvm::Function::Create(
            llvm::FunctionType::get(stringTy, {llvm::Type::getInt32Ty(ctx), stringTy, llvm::Type::getInt32Ty(ctx)},
                                    false),
            llvm::Function::ExternalLinkage, "_itoa", module);
        ltoaFunc->setCallingConv(llvm::CallingConv::C);
        ltoaFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    }
    auto* buffer =
        builder.CreateAlloca(llvm::Type::getInt8Ty(ctx), llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 20));
    builder.CreateCall(ltoaFunc, {integer, buffer, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 10)});
    return buffer;
}
} // namespace odb::ir
