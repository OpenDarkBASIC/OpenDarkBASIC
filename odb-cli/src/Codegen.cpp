#include "odb-cli/Codegen.hpp"
#include "odb-cli/AST.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"
#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/astpost/Process.hpp"
#include "odb-compiler/astpost/ResolveAndCheckTypes.hpp"
#include "odb-compiler/astpost/ResolveLabels.hpp"
#include "odb-compiler/codegen/Codegen.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Log.hpp"

#include <fstream>
#include <iostream>

static odb::codegen::OutputType outputType_ = odb::codegen::OutputType::ObjectFile;
static bool outputIsExecutable_ = true;
static std::optional<odb::codegen::TargetTriple::Arch> targetTripleArch_;
static std::optional<odb::codegen::TargetTriple::Platform> targetTriplePlatform_;

// ----------------------------------------------------------------------------
bool setOutputType(const std::vector<std::string>& args)
{
    if (args[0] == "llvm-ir")
    {
        outputType_ = odb::codegen::OutputType::LLVMIR;
    }
    else if (args[0] == "llvm-bc")
    {
        outputType_ = odb::codegen::OutputType::LLVMBitcode;
    }
    else if (args[0] == "obj" || args[0] == "exe")
    {
        outputType_ = odb::codegen::OutputType::ObjectFile;
    }

    outputIsExecutable_ = args[0] == "exe";

    return true;
}

// ----------------------------------------------------------------------------
bool setArch(const std::vector<std::string>& args)
{
    if (args[0] == "i386")
    {
        targetTripleArch_ = odb::codegen::TargetTriple::Arch::i386;
    }
    else if (args[0] == "x86_64")
    {
        targetTripleArch_ = odb::codegen::TargetTriple::Arch::x86_64;
    }
    else if (args[0] == "aarch64")
    {
        targetTripleArch_ = odb::codegen::TargetTriple::Arch::AArch64;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool setPlatform(const std::vector<std::string>& args)
{
    if (args[0] == "windows")
    {
        targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::Windows;
    }
    else if (args[0] == "macos")
    {
        targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::macOS;
    }
    else if (args[0] == "linux")
    {
        targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::Linux;
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
            targetTripleArch_ = odb::codegen::TargetTriple::Arch::i386;
        }
        else
        {
            targetTripleArch_ = odb::codegen::TargetTriple::Arch::x86_64;
        }
    }
    if (!targetTriplePlatform_)
    {
        if (getSDKType() == odb::SDKType::DarkBASIC)
        {
            targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::Windows;
        }
        else
        {
#if defined(ODBCOMPILER_PLATFORM_LINUX)
            targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::Linux;
#elif defined(ODBCOMPILER_PLATFORM_MACOS)
            targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::macOS;
#elif defined(ODBCOMPILER_PLATFORM_WIN32)
            targetTriplePlatform_ = odb::codegen::TargetTriple::Platform::Windows;
#else
#error "Unknown host platform. Add a new default target platform for the current host."
#endif
        }
    }

    odb::codegen::TargetTriple targetTriple{*targetTripleArch_, *targetTriplePlatform_};

    // Run AST post-processing (such as type checking, label resolution, etc).
    odb::astpost::ProcessGroup post;
    post.addProcess(std::make_unique<odb::astpost::ResolveAndCheckTypes>(*cmdIndex));
    post.addProcess(std::make_unique<odb::astpost::ResolveLabels>());
    if (!post.execute(ast))
    {
        return false;
    }

    // Ensure that the executable extension is .exe if Windows is the target platform.
    // TODO: Move odb::codegen to odb::codegen
    if (outputIsExecutable_ && targetTriplePlatform_ == odb::codegen::TargetTriple::Platform::Windows)
    {
        if (outputName.size() < 5 || outputName.substr(outputName.size() - 4, 4) != ".exe")
        {
            outputName += ".exe";
        }
    }

    // Generate code.
    std::unique_ptr<std::ofstream> outputFile;
    if (!outputToStdout)
    {
        outputFile = std::make_unique<std::ofstream>(outputName, std::ios::binary);
        if (!outputFile->is_open())
        {
            odb::Log::codegen(odb::Log::ERROR, "Failed to open file `%s`\n", outputName.c_str());
            return false;
        }

        odb::Log::codegen(odb::Log::INFO, "Creating output file: `%s`\n", outputName.c_str());
    }
    std::ostream& outputStream = outputToStdout ? std::cout : *outputFile;
    if (!odb::codegen::generateCode(getSDKType(), outputType_, targetTriple, outputStream, "input.dba", ast, *cmdIndex))
    {
        return false;
    }

    // If we're generating an executable, invoke the linker.
    if (outputIsExecutable_)
    {
        assert(outputType_ == odb::codegen::OutputType::ObjectFile);

        // Select a linker. By default, we choose the locally embedded LLD linker.
        std::filesystem::path linker = odb::FileSystem::getPathToSelf().parent_path() / "lld/bin";
        switch (targetTriple.platform) {
        case odb::codegen::TargetTriple::Platform::Windows:
            linker /= "lld-link";
            break;
        case odb::codegen::TargetTriple::Platform::macOS:
            linker /= "ld64.lld";
            break;
        case odb::codegen::TargetTriple::Platform::Linux:
            linker /= "ld.lld";
            break;
        }

        // Above, we generated an object file and wrote it to `outputName`, even though it is not an executable
        // yet. Here, we invoke the linker, which takes the above object file (written to `outputName`), links it, and
        // overwrites the object file with the actual executable.
        if (!odb::codegen::linkExecutable(getSDKType(), getSDKRootDir(), linker, targetTriple, {outputName}, outputName))
        {
            odb::Log::codegen(odb::Log::ERROR, "Failed to link executable.");
            return false;
        }
    }

    return true;
}
