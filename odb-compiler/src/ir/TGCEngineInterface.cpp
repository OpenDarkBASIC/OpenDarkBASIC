#include "TGCEngineInterface.hpp"
#include <filesystem>
#include <iostream>

namespace odbc::ir {
namespace {
std::string getPluginName(const std::string& pluginPath) {
    return std::filesystem::path{pluginPath}.stem().string();
}

}
TGCEngineInterface::TGCEngineInterface(llvm::Module& module) : EngineInterface(module) {
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
    loadLibraryFunc = llvm::Function::Create(
        llvm::FunctionType::get(hInstanceTy, {stringTy}, false),
        llvm::Function::ExternalLinkage, "LoadLibraryA", module);
    loadLibraryFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    loadLibraryFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetLastError from Kernel32.lib
    getLastErrorFunc = llvm::Function::Create(
        llvm::FunctionType::get(dwordTy, {}, false),
        llvm::Function::ExternalLinkage, "GetLastError", module);
    getLastErrorFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    getLastErrorFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetProcAddress from Kernel32.lib
    getProcAddrFunc = llvm::Function::Create(
        llvm::FunctionType::get(procAddrTy, {hInstanceTy, stringTy}, false),
        llvm::Function::ExternalLinkage, "GetProcAddress", module);
    getProcAddrFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    getProcAddrFunc->setCallingConv(llvm::CallingConv::X86_StdCall);

    // Declare GetModuleHandle from Kernel32.lib
    getModuleHandleFunc = llvm::Function::Create(
        llvm::FunctionType::get(hInstanceTy, {stringTy}, false),
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
            // Extra parts of GlobStruct that we don't care about has been omitted
        };
    */
    std::vector<llvm::Type*> globStructElements;
    // padding.
    globStructElements.emplace_back(llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), 48));
    // 36 HINSTANCE's for each plugin.
    for (int i = 0; i < 36; ++i) {
        globStructElements.emplace_back(hInstanceTy);
    }
    globStructTy = llvm::StructType::get(ctx, globStructElements);

    // GlobStruct consists of 36 HINSTANCE's which we need to associate with different plugins.
    // Indexes are taken from the block comment above.
    pluginGlobStructIndices["DBProSetupDebug.dll"] = 0;
    pluginGlobStructIndices["DBProBasic2DDebug.dll"] = 2;
    pluginGlobStructIndices["DBProTextDebug.dll"] = 1;
    pluginGlobStructIndices["DBProTransformsDebug.dll"] = 35;
    pluginGlobStructIndices["DBProSpritesDebug.dll"] = 3;
    pluginGlobStructIndices["DBProImageDebug.dll"] = 4;
    pluginGlobStructIndices["DBProInputDebug.dll"] = 5;
    pluginGlobStructIndices["DBProSystemDebug.dll"] = 6;
    pluginGlobStructIndices["DBProSoundDebug.dll"] = 26;
    pluginGlobStructIndices["DBProMusicDebug.dll"] = 27;
    pluginGlobStructIndices["DBProFileDebug.dll"] = 7;
    pluginGlobStructIndices["DBProFTPDebug.dll"] = 8;
    pluginGlobStructIndices["DBProMemblocksDebug.dll"] = 9;
    pluginGlobStructIndices["DBProAnimationDebug.dll"] = 11;
    pluginGlobStructIndices["DBProBitmapDebug.dll"] = 10;
    pluginGlobStructIndices["DBProMultiplayerDebug.dll"] = 12;
    pluginGlobStructIndices["DBProCameraDebug.dll"] = 14;
    pluginGlobStructIndices["DBProLightDebug.dll"] = 16;
    pluginGlobStructIndices["DBProMatrixDebug.dll"] = 15;
    pluginGlobStructIndices["DBProBasic3DDebug.dll"] = 13;
    pluginGlobStructIndices["DBProWorld3DDebug.dll"] = 17;
    pluginGlobStructIndices["DBProQ2BSPDebug.dll"] = 29;
    pluginGlobStructIndices["DBProOwnBSPDebug.dll"] = 30;
    pluginGlobStructIndices["DBProBSPCompilerDebug.dll"] = 31;
    pluginGlobStructIndices["DBProParticlesDebug.dll"] = 18;
    pluginGlobStructIndices["DBProPrimObjectDebug.dll"] = 19;
    pluginGlobStructIndices["DBProVectorsDebug.dll"] = 20;
    pluginGlobStructIndices["DBProLODTerrainDebug.dll"] = 28;
    pluginGlobStructIndices["DBProCSGDebug.dll"] = 32;
}

llvm::Function* TGCEngineInterface::generateCommandCall(const Keyword& keyword, const Keyword::Overload& overload, const std::string& functionName, llvm::FunctionType* functionType) {
    llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::InternalLinkage,
                                                      functionName, module);

    llvm::IRBuilder<> builder(module.getContext());
    llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(module.getContext(), "", function);
    builder.SetInsertPoint(basicBlock);

    // Obtain function ptr from the relevant plugin.
    callPuts(builder, builder.CreateGlobalStringPtr(functionName));
    auto commandFunction = getPluginFunction(builder, functionType, keyword.plugin,
                                             overload.symbolName, functionName + "Symbol");

    // Call it.
    std::vector<llvm::Value*> forwardedArgs;
    for (llvm::Argument& arg : function->args()) {
        forwardedArgs.emplace_back(&arg);
    }
    callPuts(builder, builder.CreateGlobalStringPtr(functionName));
    llvm::CallInst* commandResult = builder.CreateCall(commandFunction, forwardedArgs);
    callPuts(builder, builder.CreateGlobalStringPtr(functionName));
    if (functionType->getReturnType()->isVoidTy()) {
        builder.CreateRetVoid();
    } else {
        builder.CreateRet(commandResult);
    }
    return function;
}

void TGCEngineInterface::generateEntryPoint(llvm::Function* gameEntryPoint,
                                            std::vector<std::string> pluginsToLoad) {
    // Ensuring that DBProCore.dll is the first plugin.
    for (int i = 0; i < pluginsToLoad.size(); ++i) {
        if (pluginsToLoad[i] == "DBProCore.dll") {
            // Swap with front.
            std::swap(pluginsToLoad[0], pluginsToLoad[i]);
            break;
        }
    }

    // Remove plugins that we haven't used.
//    pluginsToLoad.erase(std::remove_if(pluginsToLoad.begin(), pluginsToLoad.end(), [this](const std::string& plugin) {
//        return pluginHModulePtrs.count(plugin) == 0;
//    }), pluginsToLoad.end());

    // Create main function.
    llvm::Function* entryPointFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}),
        llvm::Function::ExternalLinkage, "main", module);
    llvm::IRBuilder<> builder(ctx);

    auto callItoa = [&](llvm::Value* integer) -> llvm::Value* {
      llvm::Function* ltoaFunc = module.getFunction("_itoa");
      if (!ltoaFunc) {
          ltoaFunc = llvm::Function::Create(
              llvm::FunctionType::get(
                  stringTy, {llvm::Type::getInt32Ty(ctx), stringTy, llvm::Type::getInt32Ty(ctx)}, false),
              llvm::Function::ExternalLinkage, "_itoa", module);
          ltoaFunc->setCallingConv(llvm::CallingConv::C);
          ltoaFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
      }
      auto* buffer = builder.CreateAlloca(
          llvm::Type::getInt8Ty(ctx),
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 20));
      builder.CreateCall(
          ltoaFunc, {integer, buffer, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 10)});
      return buffer;
    };

    // Initialisation blocks.
    std::vector<llvm::BasicBlock*> pluginLoadingBlocks;
    pluginLoadingBlocks.reserve(pluginsToLoad.size());
    for (const auto& plugin : pluginsToLoad) {
        pluginLoadingBlocks.emplace_back(llvm::BasicBlock::Create(ctx, "load" + getPluginName(plugin), entryPointFunc));
    }
    llvm::BasicBlock* failedToLoadPluginsBlock = llvm::BasicBlock::Create(ctx, "failedToLoadPlugin", entryPointFunc);
    llvm::BasicBlock* initialiseEngineBlock = llvm::BasicBlock::Create(ctx, "initialiseEngine", entryPointFunc);
    llvm::Value* loadingPluginString = nullptr;
    for (int i = 0; i < pluginsToLoad.size(); ++i) {
        std::string plugin = pluginsToLoad[i];
        std::string pluginName = getPluginName(plugin);

        builder.SetInsertPoint(pluginLoadingBlocks[i]);

        if (!loadingPluginString) {
            loadingPluginString = builder.CreateGlobalStringPtr("Loading plugin");
        }

        // Call LoadLibrary and store handle.
        auto* pluginNameConstant = builder.CreateGlobalStringPtr(plugin);
        auto* libraryHModule = builder.CreateCall(
            loadLibraryFunc,
            {builder.CreateBitCast(pluginNameConstant, llvm::Type::getInt8PtrTy(ctx))});
        builder.CreateStore(libraryHModule, getOrAddPluginHModule(plugin));

        // Print that we've trying to load that plugin.
        callPuts(builder, loadingPluginString);
        callPuts(builder, pluginNameConstant);

        // Check if loaded successfully.
        auto* nextBlock = i == (pluginsToLoad.size() - 1) ? initialiseEngineBlock : pluginLoadingBlocks[i + 1];
        builder.CreateCondBr(
            builder.CreateICmpNE(libraryHModule, llvm::ConstantPointerNull::get(hInstanceTy)),
                             nextBlock, failedToLoadPluginsBlock);
    }

    // Handle plugin failure.
    builder.SetInsertPoint(failedToLoadPluginsBlock);
    callPuts(builder, builder.CreateGlobalStringPtr("Failed to load plugin. GetLastError returned"));
    callPuts(builder, callItoa(builder.CreateCall(getLastErrorFunc, {})));
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));

    // Set plugin handles in GlobStruct.
    builder.SetInsertPoint(initialiseEngineBlock);
    auto getGlobStructFunc = getPluginFunction(builder, llvm::FunctionType::get(dwordTy, false), "DBProCore.dll", "?GetGlobPtr@@YAKXZ");
    llvm::Value* globStructPtr = builder.CreateIntToPtr(builder.CreateCall(getGlobStructFunc), globStructTy->getPointerTo());
    for (const auto& [plugin, globStructIndex] : pluginGlobStructIndices) {
        auto pluginHModuleEntry = pluginHModulePtrs.find(plugin);
        // Index element in GlobStruct.
        std::vector<llvm::Value*> gepIndices;
        gepIndices.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        gepIndices.emplace_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), globStructIndex + 1));
        llvm::Value* globStructElement = builder.CreateGEP(globStructPtr, gepIndices);
        if (pluginHModuleEntry == pluginHModulePtrs.end()) {\
            builder.CreateStore(llvm::ConstantPointerNull::get(hInstanceTy), globStructElement);
        } else {
            builder.CreateStore(builder.CreateLoad(pluginHModuleEntry->second), globStructElement);
        }
    }

    // Initialise engine.
    auto passErrorPtrFunc = getPluginFunction(
        builder,
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {dwordTy->getPointerTo()}, false),
        "DBProCore.dll", "?PassErrorHandlerPtr@@YAXPAX@Z");
    auto passDLLsFunc = getPluginFunction(
        builder,
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), false),
        "DBProCore.dll", "?PassDLLs@@YAXXZ");
    auto initDisplayFunc = getPluginFunction(
        builder,
        llvm::FunctionType::get(dwordTy, {dwordTy, dwordTy, dwordTy, dwordTy, hInstanceTy, stringTy}, false),
        "DBProCore.dll", "?InitDisplay@@YAKKKKKPAUHINSTANCE__@@PAD@Z");
    auto closeDisplayFunc = getPluginFunction(
        builder,
        llvm::FunctionType::get(dwordTy, false),
        "DBProCore.dll", "?CloseDisplay@@YAKXZ");

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
        {
            llvm::ConstantInt::get(dwordTy, initialDisplayMode),
            llvm::ConstantInt::get(dwordTy, initialDisplayWidth),
            llvm::ConstantInt::get(dwordTy, initialDisplayHeight),
            llvm::ConstantInt::get(dwordTy, initialDisplayDepth),
            builder.CreateCall(getModuleHandleFunc, {llvm::ConstantPointerNull::get(stringTy)}),
            llvm::ConstantPointerNull::get(stringTy)
        });

    llvm::BasicBlock* failedToInitDisplayBlock = llvm::BasicBlock::Create(ctx, "failedToInitDisplay", entryPointFunc);
    llvm::BasicBlock* launchGameBlock = llvm::BasicBlock::Create(ctx, "launchGame", entryPointFunc);

    builder.CreateCondBr(builder.CreateICmpEQ(initDisplayResult, llvm::ConstantInt::get(dwordTy, 0)), launchGameBlock, failedToInitDisplayBlock);

    builder.SetInsertPoint(failedToInitDisplayBlock);
    callPuts(builder, builder.CreateGlobalStringPtr("Failed to init display."));
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));

    // Launch application and exit.
    builder.SetInsertPoint(launchGameBlock);
    builder.CreateCall(gameEntryPoint, {});
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
}

llvm::Value* TGCEngineInterface::getOrAddPluginHModule(const std::string& plugin) {
    auto pluginHModuleIt = pluginHModulePtrs.find(plugin);
    if (pluginHModuleIt != pluginHModulePtrs.end()) {
        return pluginHModuleIt->second;
    }

    auto* pluginHModule = new llvm::GlobalVariable(
        module, hInstanceTy, false, llvm::GlobalValue::InternalLinkage,
        llvm::ConstantPointerNull::get(hInstanceTy), getPluginName(plugin) + "HModule");
    pluginHModulePtrs.emplace(plugin, pluginHModule);
    return pluginHModule;
}

llvm::FunctionCallee TGCEngineInterface::getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy, const std::string& plugin, const std::string& symbol, const std::string& symbolStringName) {
    llvm::Value* pluginHModule = builder.CreateLoad(getOrAddPluginHModule(plugin));
    llvm::Value* procAddress = builder.CreateBitCast(
        builder.CreateCall(getProcAddrFunc, {pluginHModule, builder.CreateGlobalStringPtr(symbol, symbolStringName)}),
        functionTy->getPointerTo());
    return llvm::FunctionCallee(functionTy, procAddress);
}

void TGCEngineInterface::callPuts(llvm::IRBuilder<>& builder, llvm::Value* string) {
    llvm::Function* putsFunc = module.getFunction("puts");
    if (!putsFunc) {
        putsFunc = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {stringTy}, false),
            llvm::Function::ExternalLinkage, "puts", module);
        putsFunc->setCallingConv(llvm::CallingConv::C);
        putsFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    }
    builder.CreateCall(putsFunc, {string});
};
}