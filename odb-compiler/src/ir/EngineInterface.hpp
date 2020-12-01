#include "odbc/parsers/keywords/Keyword.hpp"

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace odbc::ir {
class EngineInterface {
public:
    explicit EngineInterface(llvm::Module& module) : module(module), ctx(module.getContext()) {}
    virtual ~EngineInterface() = default;

    virtual llvm::Function* generateCommandCall(const Keyword& keyword, const Keyword::Overload& overload, const std::string& functionName, llvm::FunctionType* functionType) = 0;
    virtual void generateEntryPoint(llvm::Function* gameEntryPoint, std::vector<std::string> pluginsToLoad) = 0;

protected:
    llvm::Module& module;
    llvm::LLVMContext& ctx;
};
}