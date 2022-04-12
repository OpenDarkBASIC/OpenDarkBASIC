#include "DBPEngineInterface.hpp"
#include "odb-compiler/parsers/PluginInfo.hpp"
#include "odb-compiler/ir/Error.hpp"

#include <filesystem>

namespace odb::ir {
DBPEngineInterface::DBPEngineInterface(llvm::Module& module, const cmd::CommandIndex& index) : EngineInterface(module, index)
{
    /*
        DBP Runtime Interface:

        void* loadPlugin(const char* pluginName);
        void* getFunctionAddress(void* plugin, const char* functionName);
        void debugPrintf(const char* fmt, ...);
        int initEngine();
        int closeEngine();
     */

    voidPtrTy = llvm::Type::getInt8PtrTy(ctx);
    charPtrTy = llvm::Type::getInt8PtrTy(ctx);
    dwordTy = llvm::Type::getInt32Ty(ctx);

    loadPluginFunc = llvm::Function::Create(llvm::FunctionType::get(voidPtrTy, {charPtrTy}, false),
                                             llvm::Function::ExternalLinkage, "loadPlugin", module);
    loadPluginFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    getFunctionAddressFunc = llvm::Function::Create(llvm::FunctionType::get(voidPtrTy, {voidPtrTy, charPtrTy}, false),
                                             llvm::Function::ExternalLinkage, "getFunctionAddress", module);
    getFunctionAddressFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    debugPrintfFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {charPtrTy}, true),
                                              llvm::Function::ExternalLinkage, "debugPrintf", module);
    debugPrintfFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    initEngineFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false),
                                            llvm::Function::ExternalLinkage, "initEngine", module);
    initEngineFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    closeEngineFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false),
                                             llvm::Function::ExternalLinkage, "closeEngine", module);
    closeEngineFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    exitProcessFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {llvm::Type::getInt32Ty(ctx)}, false),
                                             llvm::Function::ExternalLinkage, "exitProcess", module);
    exitProcessFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
    exitProcessFunc->setDoesNotReturn();
}

llvm::Function* DBPEngineInterface::generateCommandFunction(const cmd::Command& command, const std::string& functionName,
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
        builder.CreateRet(builder.CreateLoad(llvm::Type::getFloatTy(ctx), dwordAsFloatStorage));
    }
    else
    {
        builder.CreateRet(commandResult);
    }
    return function;
}

llvm::Value *DBPEngineInterface::generateMainLoopCondition(llvm::IRBuilder<>& builder) {
    // Get DBProCore library.
    const PluginInfo* dbproCore = nullptr;
    for (const PluginInfo* plugin : index.librariesAsList()) {
        if (strcmp(plugin->getName(), "DBProCore") == 0) {
            dbproCore = plugin;
            break;
        }
    }
    if (!dbproCore) {
        fatalError("DBProCore.dll missing.");
    }

    llvm::FunctionType* functionTy = llvm::FunctionType::get(dwordTy, {}, false);
    // If ProcessMessages returns 1, we exit the loop.
    auto processMessages = getPluginFunction(builder, functionTy, dbproCore, "?ProcessMessages@@YAKXZ", "ProcessMessagesSymbol");
    return builder.CreateICmpNE(builder.CreateCall(processMessages, {}), llvm::ConstantInt::get(dwordTy, 1));
}

void DBPEngineInterface::generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<PluginInfo*> pluginsToLoad)
{
    if (pluginsToLoad.empty())
    {
        fatalError("No plugins specified.");
    }

    // Ensuring that DBProCore is loaded first.
    auto isCorePlugin = [](const PluginInfo* plugin) -> bool
    {
        return strcmp(plugin->getName(), "DBProCore") == 0;
    };
    for (std::size_t i = 0; i < pluginsToLoad.size(); ++i)
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
        fatalError("DBProCore.dll is missing");
    }

    // Remove plugins that we haven't used.
    // TODO: We can't necessarily do this, as some plugins initialise different parts of the engine.
    //
    //    pluginsToLoad.erase(std::remove_if(pluginsToLoad.begin(), pluginsToLoad.end(),
    //    [this](const std::string& plugin) {
    //        return pluginHandlePtrs.count(plugin) == 0;
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
        pluginLoadingBlocks.emplace_back(llvm::BasicBlock::Create(ctx, "load" + std::string{plugin->getName()}, entryPointFunc));
    }
    llvm::BasicBlock* initEngineBlock = llvm::BasicBlock::Create(ctx, "initEngine", entryPointFunc);
    llvm::BasicBlock* launchGameBlock = llvm::BasicBlock::Create(ctx, "launchGame", entryPointFunc);
    llvm::BasicBlock* returnSuccessBlock = llvm::BasicBlock::Create(ctx, "returnSuccess", entryPointFunc);
    llvm::BasicBlock* returnFailureBlock = llvm::BasicBlock::Create(ctx, "returnFailure", entryPointFunc);
    
    // Load plugins.
    for (std::size_t i = 0; i < pluginsToLoad.size(); ++i)
    {
        PluginInfo* plugin = pluginsToLoad[i];
        std::string pluginName = plugin->getName();
        std::string pluginPath = std::filesystem::path{plugin->getPath()}.filename().string();

        builder.SetInsertPoint(pluginLoadingBlocks[i]);

        auto* pluginNameConstant = builder.CreateGlobalStringPtr(pluginPath);

        // Load the library and store the handle.
        auto* libraryHandle = builder.CreateCall(
            loadPluginFunc, {builder.CreateBitCast(pluginNameConstant, llvm::Type::getInt8PtrTy(ctx))});
        builder.CreateStore(libraryHandle, getOrAddPluginHandleVar(plugin));

        // Check if loaded successfully.
        auto* nextBlock = i == (pluginsToLoad.size() - 1) ? initEngineBlock : pluginLoadingBlocks[i + 1];
        builder.CreateCondBr(builder.CreateICmpNE(libraryHandle, llvm::ConstantPointerNull::get(voidPtrTy)),
                             nextBlock, returnFailureBlock);
    }

    // Init engine.
    builder.SetInsertPoint(initEngineBlock);
    auto* initEngineResult = builder.CreateCall(initEngineFunc, {});
    builder.CreateCondBr(builder.CreateICmpEQ(initEngineResult, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0)), launchGameBlock, returnFailureBlock);

    // Launch application.
    builder.SetInsertPoint(launchGameBlock);
    builder.CreateCall(gameEntryPoint, {});

    // Clean up.
    auto* closeEngineResult = builder.CreateCall(closeEngineFunc, {});
    builder.CreateCondBr(builder.CreateICmpEQ(closeEngineResult, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0)), returnSuccessBlock, returnFailureBlock);

    // Success.
    builder.SetInsertPoint(returnSuccessBlock);
    builder.CreateCall(exitProcessFunc, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0)});
    builder.CreateUnreachable();

    // Failure.
    builder.SetInsertPoint(returnFailureBlock);
    builder.CreateCall(exitProcessFunc, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1)});
    builder.CreateUnreachable();
}

llvm::Value* DBPEngineInterface::getOrAddPluginHandleVar(const PluginInfo* plugin)
{
    auto pluginName = plugin->getName();

    auto pluginHandleIt = pluginHandlePtrs.find(pluginName);
    if (pluginHandleIt != pluginHandlePtrs.end())
    {
        return pluginHandleIt->second;
    }

    auto* pluginHandle = new llvm::GlobalVariable(module, voidPtrTy, false, llvm::GlobalValue::InternalLinkage,
                                                   llvm::ConstantPointerNull::get(voidPtrTy), std::string{pluginName} + "Handle");
    pluginHandlePtrs.emplace(pluginName, pluginHandle);
    return pluginHandle;
}

llvm::FunctionCallee DBPEngineInterface::getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                                           const PluginInfo* library, const std::string& symbol,
                                                           const std::string& symbolStringName)
{
    llvm::Value* pluginHandle = builder.CreateLoad(getOrAddPluginHandleVar(library));
    llvm::CallInst* procAddress =
        builder.CreateCall(getFunctionAddressFunc, {pluginHandle, builder.CreateGlobalStringPtr(symbol, symbolStringName)});
    return llvm::FunctionCallee(functionTy, builder.CreateBitCast(procAddress, functionTy->getPointerTo()));
}
} // namespace odb::ir
