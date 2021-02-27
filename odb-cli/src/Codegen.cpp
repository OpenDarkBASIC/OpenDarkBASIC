#include "odb-cli/Codegen.hpp"
#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"
#include "odb-compiler/ir/Codegen.hpp"
#include "odb-compiler/ir/Node.hpp"
#include "odb-compiler/ir/SemanticChecker.hpp"
#include "odb-sdk/Log.hpp"

#include <fstream>
#include <iostream>

static odb::ir::OutputType outputType_ = odb::ir::OutputType::ObjectFile;
static bool outputIsExecutable_ = false;
static std::optional<odb::ir::TargetTriple::Arch> targetTripleArch_;
static std::optional<odb::ir::TargetTriple::Platform> targetTriplePlatform_;

// ----------------------------------------------------------------------------
bool setOutputType(const std::vector<std::string>& args)
{
    if (args[0] == "llvm-ir")
    {
        outputType_ = odb::ir::OutputType::LLVMIR;
    }
    else if (args[0] == "llvm-bc")
    {
        outputType_ = odb::ir::OutputType::LLVMBitcode;
    }
    else if (args[0] == "obj" || args[0] == "exe")
    {
        outputType_ = odb::ir::OutputType::ObjectFile;
    }

    outputIsExecutable_ = args[0] == "exe";

    return true;
}

// ----------------------------------------------------------------------------
bool setArch(const std::vector<std::string>& args)
{
    if (args[0] == "i386")
    {
        targetTripleArch_ = odb::ir::TargetTriple::Arch::i386;
    }
    else if (args[0] == "x86_64")
    {
        targetTripleArch_ = odb::ir::TargetTriple::Arch::x86_64;
    }
    else if (args[0] == "aarch64")
    {
        targetTripleArch_ = odb::ir::TargetTriple::Arch::AArch64;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool setPlatform(const std::vector<std::string>& args)
{
    if (args[0] == "windows")
    {
        targetTriplePlatform_ = odb::ir::TargetTriple::Platform::Windows;
    }
    else if (args[0] == "macos")
    {
        targetTriplePlatform_ = odb::ir::TargetTriple::Platform::macOS;
    }
    else if (args[0] == "linux")
    {
        targetTriplePlatform_ = odb::ir::TargetTriple::Platform::Linux;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool output(const std::vector<std::string>& args)
{
    std::string outputName = args[0];
    bool outputToStdout = outputName == "-";

    auto* cmdIndex = getCommandIndex();
    auto* ast = getAST();

    // Set default target triple if they are not specified by the user.
    if (!targetTripleArch_)
    {
        if (getSDKType() == odb::SDKType::DarkBASIC)
        {
            targetTripleArch_ = odb::ir::TargetTriple::Arch::i386;
        }
        else
        {
            targetTripleArch_ = odb::ir::TargetTriple::Arch::x86_64;
        }
    }
    if (!targetTriplePlatform_)
    {
        if (getSDKType() == odb::SDKType::DarkBASIC)
        {
            targetTriplePlatform_ = odb::ir::TargetTriple::Platform::Windows;
        }
        else
        {
#if defined(ODBCOMPILER_PLATFORM_LINUX)
            targetTriplePlatform_ = odb::ir::TargetTriple::Platform::Linux;
#elif defined(ODBCOMPILER_PLATFORM_MACOS)
            targetTriplePlatform_ = odb::ir::TargetTriple::Platform::macOS;
#elif defined(ODBCOMPILER_PLATFORM_WIN32)
            targetTriplePlatform_ = odb::ir::TargetTriple::Platform::Windows;
#else
#error "Unknown host platform. Add a new default target platform for the current host."
#endif
        }
    }

    // Generate IR program, then generate code.
    auto program = odb::ir::runSemanticChecks(ast, *cmdIndex);
    if (!program)
    {
        return false;
    }

    // Generate code.
    std::filesystem::path codegenOutputFilename = outputName;
    if (outputIsExecutable_)
    {
        codegenOutputFilename.filename() = "_" + codegenOutputFilename.filename().string() + ".o";
    }
    std::unique_ptr<std::ofstream> outputFile;
    if (!outputToStdout)
    {
        outputFile = std::make_unique<std::ofstream>(codegenOutputFilename, std::ios::binary);
        if (!outputFile->is_open())
        {
            odb::Log::codegen(odb::Log::ERROR, "Failed to open file `%s`\n", codegenOutputFilename.c_str());
            return false;
        }

        odb::Log::codegen(odb::Log::INFO, "Creating output file: `%s`\n", codegenOutputFilename.c_str());
    }
    std::ostream& outputStream = outputToStdout ? std::cout : *outputFile;
    if (!odb::ir::generateCode(getSDKType(), outputType_,
                               odb::ir::TargetTriple{*targetTripleArch_, *targetTriplePlatform_}, outputStream,
                               "input.dba", *program, *cmdIndex))
    {
        return false;
    }

    if (outputIsExecutable_)
    {
        // We need to link the generated object file to the runtime to create an executable.
        if (targetTriplePlatform_ == odb::ir::TargetTriple::Platform::Windows)
        {
            if (outputName.size() < 5 || outputName.substr(outputName.size() - 4, 4) != ".exe")
            {
                outputName += ".exe";
            }
        }

        if (!odb::ir::linkExecutable(*targetTriplePlatform_,
                                     {codegenOutputFilename, getSDKRootDir() / "odb-runtime-dbp.lib",
                                      getSDKRootDir() / "odb-runtime-dbp-prelude.lib"},
                                     outputName))
        {
            odb::Log::codegen(odb::Log::ERROR, "Failed to link executable.");
            return false;
        }
    }

    return true;
}
