#include "DBPEngineInterface.hpp"
#include "odb-compiler/parsers/DynamicLibData.hpp"

#include <filesystem>

namespace odb::ir {
namespace {
std::string getPluginName(const DynamicLibData* library)
{
    return std::filesystem::path{library->getFilename()}.stem().string();
}
} // namespace

DBPEngineInterface::DBPEngineInterface(llvm::Module& module) : EngineInterface(module)
{
    dwordTy = llvm::Type::getInt8PtrTy(ctx);

    /*
        DBP Runtime Interface:

        void* loadPlugin(const char* pluginName);
        void* getFunctionAddress(void* plugin, const char* functionName);
        void debugPrintf(const char* fmt, ...);
        int initialiseEngine();
     */

    voidPtrTy = llvm::Type::getInt8PtrTy(ctx);
    charPtrTy = llvm::Type::getInt8PtrTy(ctx);

    loadPluginFunc = llvm::Function::Create(llvm::FunctionType::get(voidPtrTy, {charPtrTy}, false),
                                             llvm::Function::ExternalLinkage, "loadPlugin", module);
    loadPluginFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    getFunctionAddressFunc = llvm::Function::Create(llvm::FunctionType::get(voidPtrTy, {voidPtrTy, charPtrTy}, false),
                                             llvm::Function::ExternalLinkage, "getFunctionAddress", module);
    getFunctionAddressFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    debugPrintfFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {charPtrTy}, true),
                                              llvm::Function::ExternalLinkage, "debugPrintf", module);
    debugPrintfFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);

    initialiseEngineFunc = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(ctx), {}, false),
                                             llvm::Function::ExternalLinkage, "initialiseEngine", module);
    initialiseEngineFunc->setDLLStorageClass(llvm::Function::DLLImportStorageClass);
}

llvm::Function* DBPEngineInterface::generateCommandCall(const cmd::Command& command, const std::string& functionName,
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

void DBPEngineInterface::generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<DynamicLibData*> pluginsToLoad)
{
    if (pluginsToLoad.empty())
    {
        // TODO: Fatal error.
        fprintf(stderr, "FATAL ERROR: No plugins specified.\n");
        std::terminate();
    }

    // Ensuring that DBProCore.dll is the first plugin.
    auto isCorePlugin = [](const DynamicLibData* library) -> bool
    {
        return std::filesystem::path{library->getFilename()}.stem() == "DBProCore";
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
        // TODO: Fatal error.
        fprintf(stderr, "FATAL ERROR: DBProCore.dll is missing.\n");
        std::terminate();
    }

    // Remove plugins that we haven't used.
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
        pluginLoadingBlocks.emplace_back(llvm::BasicBlock::Create(ctx, "load" + getPluginName(plugin), entryPointFunc));
    }
    llvm::BasicBlock* initialiseEngineBlock = llvm::BasicBlock::Create(ctx, "initialiseEngine", entryPointFunc);
    llvm::BasicBlock* failedToInitialiseEngineBlock = llvm::BasicBlock::Create(ctx, "failedToInitialiseEngine", entryPointFunc);
    llvm::BasicBlock* launchGameBlock = llvm::BasicBlock::Create(ctx, "launchGame", entryPointFunc);
    
    // Load plugins.
    for (std::size_t i = 0; i < pluginsToLoad.size(); ++i)
    {
        DynamicLibData* plugin = pluginsToLoad[i];
        std::string pluginName = getPluginName(plugin);
        std::string pluginPath = std::filesystem::path{plugin->getFilename()}.filename().string();

        builder.SetInsertPoint(pluginLoadingBlocks[i]);

        auto* pluginNameConstant = builder.CreateGlobalStringPtr(pluginPath);

        // Load the library and store the handle.
        auto* libraryHandle = builder.CreateCall(
            loadPluginFunc, {builder.CreateBitCast(pluginNameConstant, llvm::Type::getInt8PtrTy(ctx))});
        builder.CreateStore(libraryHandle, getOrAddPluginHandleVar(plugin));

        // Check if loaded successfully.
        auto* nextBlock = i == (pluginsToLoad.size() - 1) ? initialiseEngineBlock : pluginLoadingBlocks[i + 1];
        builder.CreateCondBr(builder.CreateICmpNE(libraryHandle, llvm::ConstantPointerNull::get(voidPtrTy)),
                             nextBlock, failedToInitialiseEngineBlock);
    }

    // Initialise engine.
    builder.SetInsertPoint(initialiseEngineBlock);
    auto* initialiseEngineResult = builder.CreateCall(initialiseEngineFunc, {});
    builder.CreateCondBr(builder.CreateICmpEQ(initialiseEngineResult, llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0)), launchGameBlock, failedToInitialiseEngineBlock);

    // Failure.
    builder.SetInsertPoint(failedToInitialiseEngineBlock);
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 1));

    // Launch application and exit.
    builder.SetInsertPoint(launchGameBlock);
    builder.CreateCall(gameEntryPoint, {});
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
}

llvm::Value* DBPEngineInterface::getOrAddPluginHandleVar(const DynamicLibData* library)
{
    auto pluginName = getPluginName(library);

    auto pluginHandleIt = pluginHandlePtrs.find(pluginName);
    if (pluginHandleIt != pluginHandlePtrs.end())
    {
        return pluginHandleIt->second;
    }

    auto* pluginHandle = new llvm::GlobalVariable(module, voidPtrTy, false, llvm::GlobalValue::InternalLinkage,
                                                   llvm::ConstantPointerNull::get(voidPtrTy), pluginName + "Handle");
    pluginHandlePtrs.emplace(pluginName, pluginHandle);
    return pluginHandle;
}

llvm::FunctionCallee DBPEngineInterface::getPluginFunction(llvm::IRBuilder<>& builder, llvm::FunctionType* functionTy,
                                                           const DynamicLibData* library, const std::string& symbol,
                                                           const std::string& symbolStringName)
{
    llvm::Value* pluginHandle = builder.CreateLoad(getOrAddPluginHandleVar(library));
    llvm::CallInst* procAddress =
        builder.CreateCall(getFunctionAddressFunc, {pluginHandle, builder.CreateGlobalStringPtr(symbol, symbolStringName)});
    return llvm::FunctionCallee(functionTy, builder.CreateBitCast(procAddress, functionTy->getPointerTo()));
}
} // namespace odb::ir
