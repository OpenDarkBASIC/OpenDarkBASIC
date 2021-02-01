#include "odb-compiler/ir/Codegen.hpp"

#include "codegen/CodeGenerator.hpp"
#include "codegen/LLVM.hpp"
#include "codegen/TGCEngineInterface.hpp"

#include <iostream>

namespace odb::ir {
void generateCode(OutputType type, std::ostream& os, const std::string& moduleName, Program& program,
                  const cmd::CommandIndex& cmdIndex)
{
    llvm::LLVMContext context;
    llvm::Module module(moduleName, context);
    TGCEngineInterface engineInterface(module);
    CodeGenerator gen(module, engineInterface);
    gen.generateModule(program, cmdIndex.librariesAsList());

    // If we are emitting LLVM IR or Bitcode, return early.
    if (type == OutputType::LLVMIR)
    {
        llvm::raw_os_ostream outputStream(os);
        module.print(outputStream, nullptr);
        return;
    }
    else if (type == OutputType::LLVMBitcode)
    {
        llvm::raw_os_ostream outputStream(os);
        llvm::WriteBitcodeToFile(module, outputStream);
        return;
    }

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();

    // Lookup target machine.
    auto targetTriple = "i386-pc-windows-msvc";
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target)
    {
        // TODO: Return error.
        std::cerr << "Unknown target triple. Error: " << error;
        std::terminate();
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    llvm::TargetMachine* targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, {});
    module.setDataLayout(targetMachine->createDataLayout());
    module.setTargetTriple(targetTriple);

    llvm::SmallVector<char, 0> outputFileBuffer;

    if (type == OutputType::ObjectFile)
    {
        // Emit object file to buffer.
        llvm::raw_svector_ostream objectFileStream(outputFileBuffer);
        llvm::legacy::PassManager pass;
        if (targetMachine->addPassesToEmitFile(pass, objectFileStream, nullptr, llvm::CGFT_ObjectFile))
        {
            std::cerr << "TargetMachine can't emit a file of this type";
            std::terminate();
        }
        pass.run(module);
    }
    else if (type == OutputType::Executable)
    {
        // TODO: Call LLD linker to create an executable.
    }

    // Flush buffer to stream.
    os.write(outputFileBuffer.data(), outputFileBuffer.size());
    os.flush();
}
} // namespace odb::ir