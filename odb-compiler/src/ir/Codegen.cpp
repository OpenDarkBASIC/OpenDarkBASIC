#include "odb-compiler/ir/Codegen.hpp"

#include "codegen/CodeGenerator.hpp"
#include "codegen/DBPEngineInterface.hpp"
#include "codegen/LLVM.hpp"
#include "codegen/ODBEngineInterface.hpp"

#include <iostream>

#include <lld/Common/Driver.h>

namespace odb::ir {
bool generateCode(SDKType sdk_type, OutputType outputType, TargetTriple targetTriple, std::ostream& os,
                  const std::string& moduleName, Program& program, const cmd::CommandIndex& cmdIndex)
{
    llvm::LLVMContext context;
    llvm::Module module(moduleName, context);

    // Generate the module.
    {
        std::unique_ptr<EngineInterface> engineInterface;
        switch (sdk_type)
        {
        case SDKType::DarkBASIC:
            engineInterface = std::make_unique<DBPEngineInterface>(module);
            break;
        case SDKType::ODB:
            engineInterface = std::make_unique<ODBEngineInterface>(module);
            break;
        default:
            Log::info.print("Code generation not implemented for the specified SDK type.");
            return false;
        }
        CodeGenerator gen(module, *engineInterface);
        if (!gen.generateModule(program, cmdIndex.librariesAsList()))
        {
            return false;
        }
    }

    // If we are emitting LLVM IR or Bitcode, return early.
    if (outputType == OutputType::LLVMIR)
    {
        llvm::raw_os_ostream outputStream(os);
        module.print(outputStream, nullptr);
        return true;
    }
    else if (outputType == OutputType::LLVMBitcode)
    {
        llvm::raw_os_ostream outputStream(os);
        llvm::WriteBitcodeToFile(module, outputStream);
        return true;
    }

    assert(outputType == OutputType::ObjectFile);

    static std::once_flag initLLVMBackends;
    std::call_once(initLLVMBackends,
                   []
                   {
                       LLVMInitializeX86TargetInfo();
                       LLVMInitializeX86Target();
                       LLVMInitializeX86TargetMC();
                       LLVMInitializeX86AsmPrinter();
                       LLVMInitializeAArch64TargetInfo();
                       LLVMInitializeAArch64Target();
                       LLVMInitializeAArch64TargetMC();
                       LLVMInitializeAArch64AsmPrinter();
                   });

    // Lookup target machine.
    std::string llvmTargetTriple = targetTriple.getLLVMTargetTriple();
    if (sdk_type == SDKType::DarkBASIC)
    {
        // Only the i386-pc-windows-msvc target triple is supported.
        if (targetTriple.arch != TargetTriple::Arch::i386 || targetTriple.platform != TargetTriple::Platform::Windows)
        {
            Log::info.print(
                "Unsupported platform and arch. Only i386 on Windows is supported when working with the DBP SDK type.");
            return false;
        }
    }
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(llvmTargetTriple, error);
    if (!target)
    {
        Log::info.print("Unknown target triple: %s", error.c_str());
        return false;
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    llvm::TargetMachine* targetMachine = target->createTargetMachine(llvmTargetTriple, cpu, features, opt, {});
    module.setDataLayout(targetMachine->createDataLayout());
    module.setTargetTriple(llvmTargetTriple);

    llvm::SmallVector<char, 0> outputFileBuffer;

    // Emit object file to buffer.
    llvm::raw_svector_ostream objectFileStream(outputFileBuffer);
    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, objectFileStream, nullptr, llvm::CGFT_ObjectFile))
    {
        Log::info.print("llvm::TargetMachine can't emit a file of this type");
        return false;
    }
    pass.run(module);

    // Flush buffer to stream.
    os.write(outputFileBuffer.data(), outputFileBuffer.size());
    os.flush();

    return true;
}

bool linkExecutable(TargetTriple::Platform platform, const std::vector<std::string>& inputFilenames,
                    std::string& outputFilename)
{
    if (platform == TargetTriple::Platform::Windows)
    {
        std::string outFlag = "/out:" + outputFilename;

        std::vector<const char*> args;
        args.emplace_back("lld");
        for (const auto& inputs : inputFilenames)
        {
            args.emplace_back(inputs.c_str());
        }
        args.emplace_back(outFlag.c_str());
        args.emplace_back("/nodefaultlib");
        args.emplace_back("/entry:main");
        args.emplace_back("/subsystem:windows");
        args.emplace_back("/machine:x86");
        llvm::raw_os_ostream outputStream{std::cout};
        llvm::raw_os_ostream errorStream{std::cerr};
        return lld::coff::link(args, false, outputStream, errorStream);
    }
    else
    {
        // TODO: Implement ELF linking.
        return false;
    }
}
} // namespace odb::ir