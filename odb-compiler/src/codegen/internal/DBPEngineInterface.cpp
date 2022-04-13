#include "DBPEngineInterface.hpp"
#include "odb-compiler/parsers/PluginInfo.hpp"
#include "odb-compiler/codegen/Error.hpp"
#include "odb-compiler/ast/Type.hpp"

#include <filesystem>

namespace odb::codegen {
namespace {
// Returns the DBP engine encoded type ID.
uint32_t getTypeID(ast::Type ty)
{
    // Source: CStructTable::SetStructDefaults
    if (!ty.isBuiltinType())
    {
        // TODO: Implement UDTs
        return 0;
    }

    // TODO: For arrays: add 100
    /*
        SetStruct(1, "integer",					'L', 4);
        AddStruct(2, "float",					'F', 4);
        AddStruct(3, "string",					'S', 4);
        AddStruct(4, "boolean",					'B', 1);
        AddStruct(5, "byte",					'Y', 1);
        AddStruct(6, "word",					'W', 2);
        AddStruct(7, "dword",					'D', 4);
        AddStruct(8, "double float",			'O', 8);
        AddStruct(9, "double integer",			'R', 8);
     */
    switch (*ty.getBuiltinType()) {
        case ast::BuiltinType::DoubleInteger:
            return 9;
        case ast::BuiltinType::Integer:
            return 1;
        case ast::BuiltinType::Dword:
            return 7;
        case ast::BuiltinType::Word:
            return 6;
        case ast::BuiltinType::Byte:
            return 5;
        case ast::BuiltinType::Boolean:
            return 4;
        case ast::BuiltinType::DoubleFloat:
            return 8;
        case ast::BuiltinType::Float:
            return 2;
        case ast::BuiltinType::String:
            return 3;
        case ast::BuiltinType::Complex:
        case ast::BuiltinType::Mat2x2:
        case ast::BuiltinType::Mat2x3:
        case ast::BuiltinType::Mat2x4:
        case ast::BuiltinType::Mat3x2:
        case ast::BuiltinType::Mat3x3:
        case ast::BuiltinType::Mat3x4:
        case ast::BuiltinType::Mat4x2:
        case ast::BuiltinType::Mat4x3:
        case ast::BuiltinType::Mat4x4:
        case ast::BuiltinType::Quat:
        case ast::BuiltinType::Vec2:
        case ast::BuiltinType::Vec3:
                case ast::BuiltinType::Vec4:
            return 0; // TODO: implement extended types.
    }
}
}
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

llvm::Value* DBPEngineInterface::generateAllocateArray(llvm::IRBuilder<>& builder, ast::Type arrayElementTy, std::vector<llvm::Value*> dims) {
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

    // DimDDD has 11 parameters (old array ptr, type and size of element, 9 dimensions)
    std::vector<llvm::Type*> paramTypes;
    paramTypes.resize(11);
    for (int i = 0; i < 11; ++i) {
        paramTypes[i] = dwordTy;
    }
    llvm::FunctionType* functionTy = llvm::FunctionType::get(dwordTy, paramTypes, false);
    auto dimArray = getPluginFunction(builder, functionTy, dbproCore, "?DimDDD@@YAKKKKKKKKKKKK@Z", "DimDDDSymbol");

    // Call function.
    std::vector<llvm::Value*> params;
    // TODO: old array ptr, null for now
    params.push_back(llvm::ConstantInt::get(dwordTy, 0));
    // Encode type and size of element. Store type ID in first 12 bits, and size is shifted by 12 bits.
    uint32_t encodedTypeAndSize = getTypeID(arrayElementTy) + (arrayElementTy.size() << 12);
    params.push_back(llvm::ConstantInt::get(dwordTy, encodedTypeAndSize));
    // Encode dimensions
    size_t num_dimensions = dims.size();
    if (num_dimensions > 9) {
        // TODO; Consider having a 'IRResult<ReturnType>' return type wrapper that typedefs std::expected, to propagate errors.
        fatalError("Can't create an array with more than 9 dimensions.");
    }
    for (size_t i = 0; i < num_dimensions; ++i) {
        params.push_back(dims[i]);
    }
    for (size_t i = num_dimensions; i < 9; ++i) {
        // Remaining unspecified dimensions are 0.
        params.push_back(llvm::ConstantInt::get(dwordTy, 0));
    }

    // Function returns DWORD, cast to array type using inttoptr.
    llvm::Value* arrayPtrDword = builder.CreateCall(dimArray, params);//, llvm::ConstantInt::get(dwordTy, 1));
    return builder.CreateIntToPtr(arrayPtrDword, voidPtrTy);

    /*

    Structure of an array of length N (source: 'CreateArray' in DBDLLCore.cpp):

    * 56 bytes: Header (14 DWORDs at 4 bytes each)
        header[0] = d1
        header[1] = header[0] * d2
        header[2] = header[1] * d3
        header[3] = header[2] * d4
        header[4] = header[3] * d5
        header[5] = header[4] * d6
        header[6] = header[5] * d7
        header[7] = header[6] * d8
        header[8] = header[7] * d9
        header[10] = N
        header[11] = size of one item, encoded in the 2nd parameter of DimDDD
        header[12] = type ID of each item, encoded in the 2nd parameter of DimDDD;
        header[13] = the internal index of this array (what 'array()' means), defaults to 0

    * N*4 bytes: Ref Table, 4 bytes for each element
        * The 'ref table' is an array of pointers into the data section of the DBP array
            // Create Ref Table
            LPSTR pDataPointer = pData;
            for(DWORD r=0; r<dwSizeOfArray; r++)
            {
                pRef[r] = (DWORD)pDataPointer;
                pDataPointer+=dwSizeOfOneDataItem;
            }

    * N bytes: DataBlockFlag Table, 1 byte for each element
        * All flags set to 1.

    * N * dwSizeOfOneDataItem bytes: Data. The Ref Table points into each element in this data section.
        * Zero'd in CreateArray.

    The pointer to the ref table is returned (pointer returned by malloc + 56)

    */

    /*
    DARKSDK DWORD CreateArray(DWORD dwSizeOfArray, DWORD dwSizeOfOneDataItem, DWORD dwTypeValueOfOneDataItem)
    {
        // Calculate Total Size of Array
        DWORD dwHeaderSizeInBytes = HEADERSIZEINBYTES;
        DWORD dwDimSizeBytes = 40;
        DWORD dwRefSizeInBytes = dwSizeOfArray * 4;
        DWORD dwFlagSizeInBytes = dwSizeOfArray * 1;
        DWORD dwDataSizeInBytes = dwSizeOfArray * dwSizeOfOneDataItem;

        // Total Size
        DWORD dwTotalSize = dwHeaderSizeInBytes + dwRefSizeInBytes + dwFlagSizeInBytes + dwDataSizeInBytes;

        // Create Array Memory
        LPSTR pArrayPtr = new char[dwTotalSize];
        memset(pArrayPtr, 0, sizeof(pArrayPtr));

        // Derive Pointers into Array
        DWORD* pHeader	= (DWORD*)(pArrayPtr);
        DWORD* pRef		= (DWORD*)(pArrayPtr+dwHeaderSizeInBytes);
        LPSTR  pFlag	= (LPSTR )(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes);
        LPSTR  pData	= (LPSTR )(pArrayPtr+dwHeaderSizeInBytes+dwRefSizeInBytes+dwFlagSizeInBytes);

        // Create Header
        for(DWORD d=0; d<=9; d++) pHeader[0]=0;
        pHeader[10]=dwSizeOfArray;
        pHeader[11]=dwSizeOfOneDataItem;
        pHeader[12]=dwTypeValueOfOneDataItem;
        pHeader[13]=0;

        // Create Ref Table
        LPSTR pDataPointer = pData;
        for(DWORD r=0; r<dwSizeOfArray; r++)
        {
            pRef[r] = (DWORD)pDataPointer;
            pDataPointer+=dwSizeOfOneDataItem;
        }

        // Create DataBlockFlag Table (all flags to 1)
        memset(pFlag, 1, dwSizeOfArray);

        // Clear DataBlock Memory
        DWORD dwTotalDataSize = dwSizeOfArray * dwSizeOfOneDataItem;
        memset(pData, 0, dwTotalDataSize);

        // Advance ArrayPtr to First Byte in RefTable
        pArrayPtr+=dwHeaderSizeInBytes;

        // Return ArrayPtr
        return (DWORD)pArrayPtr;
    }

     */
}

llvm::Value* DBPEngineInterface::generateIndexArray(llvm::IRBuilder<>& builder, llvm::Type* arrayElementPtrTy, llvm::Value *arrayPtr, std::vector<llvm::Value*> dims) {
    // To obtain the memory location of the data itself, we use the array pointer to index into the ref table and return
    // the ref table entry.
    llvm::Type* int32PtrTy = llvm::IntegerType::getInt32PtrTy(ctx);
    llvm::Value* refTable = builder.CreateBitCast(arrayPtr, int32PtrTy);
    if (dims.size() > 1) {
        fatalError("Can't index arrays with more than 1 dimension.");
    }
    llvm::Value* memoryAddressAsDword = builder.CreateLoad(llvm::IntegerType::getInt32Ty(ctx), builder.CreateGEP(llvm::IntegerType::getInt32Ty(ctx), refTable, dims));
    llvm::Value* elementPtr = builder.CreateIntToPtr(memoryAddressAsDword, arrayElementPtrTy);
    return elementPtr;
}

void DBPEngineInterface::generateFreeArray(llvm::IRBuilder<>& builder, llvm::Value *arrayPtr) {
    (void)(builder, arrayPtr);
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
} // namespace odb::codegen
