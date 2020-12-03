#include "EngineInterface.hpp"

namespace odb::ir {
class TGCEngineInterface : public EngineInterface
{
public:
    explicit TGCEngineInterface(llvm::Module& module);

    llvm::Function* generateCommandCall(const Keyword& keyword, const Keyword::Overload& overload,
                                        const std::string& functionName,
                                        llvm::FunctionType* functionType) override;
    void generateEntryPoint(llvm::Function* gameEntryPoint,
                            std::vector<std::string> pluginsToLoad) override;

private:
    llvm::PointerType* hInstanceTy;
    llvm::PointerType* stringTy;
    llvm::PointerType* procAddrTy;
    llvm::Type* dwordTy;
    llvm::StructType* globStructTy;

    llvm::Function* loadLibraryFunc;
    llvm::Function* getLastErrorFunc;
    llvm::Function* getProcAddrFunc;
    llvm::Function* getModuleHandleFunc;

    std::unordered_map<std::string, llvm::Value*> pluginHModulePtrs;
    std::unordered_map<std::string, int> pluginGlobStructIndices;

    llvm::Value* getOrAddPluginHModule(const std::string& plugin);
    llvm::FunctionCallee getPluginFunction(llvm::IRBuilder<>& builder,
                                           llvm::FunctionType* functionTy,
                                           const std::string& plugin, const std::string& symbol,
                                           const std::string& symbolStringName = "");
    void printString(llvm::IRBuilder<>& builder, llvm::Value* string);
    llvm::Value* convertIntegerToString(llvm::IRBuilder<>& builder, llvm::Value* integer);
};
}  // namespace odb::ir