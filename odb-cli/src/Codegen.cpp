#include "odb-cli/AST.hpp"
#include "odb-cli/Codegen.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-compiler/codegen/ir.h"
#include "odb-compiler/codegen/target.h"
#include "odb-compiler/link/link.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-sdk/log.h"
}

static bool             outputIsExecutable_ = true;
static enum target_arch arch_ = TARGET_x86_64;
#if defined(ODBSDK_PLATFORM_LINUX)
static enum target_platform platform_ = TARGET_LINUX;
#else
static enum target_platform platform_ = TARGET_WINDOWS;
#endif

// ----------------------------------------------------------------------------
bool
setArch(const std::vector<std::string>& args)
{
    if (args[0] == "i386")
        arch_ = TARGET_i386;
    else if (args[0] == "x86_64")
        arch_ = TARGET_x86_64;
    else if (args[0] == "aarch64")
        arch_ = TARGET_AArch64;
    else
        log_err("", "Unknown architecture {quote:%s}\n", args[0].c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool
setPlatform(const std::vector<std::string>& args)
{
    if (args[0] == "windows")
        platform_ = TARGET_WINDOWS;
    else if (args[0] == "macos")
        platform_ = TARGET_MACOS;
    else if (args[0] == "linux")
        platform_ = TARGET_LINUX;
    else
        log_err("", "Unknown platform {quote:%s}\n", args[0].c_str());

    return true;
}

// ----------------------------------------------------------------------------
bool
output(const std::vector<std::string>& args)
{
    std::string outputName = args[0];

    std::string       srcfile = getSourceFilename();
    std::string       objfile = srcfile + ".o";
    std::string       modname = srcfile.substr(0, srcfile.rfind("."));
    struct ir_module* ir = ir_alloc(modname.c_str());
    ir_translate_ast(
        ir,
        getAST(),
        getSDKType(),
        getCommandList(),
        getSourceFilename(),
        getSource());
    ir_optimize(ir);
    ir_compile(ir, objfile.c_str(), arch_, platform_);
    ir_free(ir);

    struct used_cmds_hm* used_cmds_hm;
    used_cmds_init(&used_cmds_hm);
    used_cmds_append(&used_cmds_hm, getAST());
    struct cmd_ids* used_cmds_list = used_cmds_finalize(used_cmds_hm);

    ir = ir_alloc("odbruntime");
    ir_create_runtime(
        ir,
        getPluginList(),
        getCommandList(),
        used_cmds_list,
        modname.c_str(),
        getSDKType(),
        arch_,
        platform_);
    ir_compile(ir, "odbruntime.o", arch_, platform_);
    ir_free(ir);

    cmd_ids_deinit(used_cmds_list);

    const char* objfiles[] = {objfile.c_str(), "odbruntime.o"};
    odb_link(objfiles, 2, outputName.c_str(), arch_, platform_);

    return true;
}

// ----------------------------------------------------------------------------
enum target_arch
getTargetArch(void)
{
    return arch_;
}

// ----------------------------------------------------------------------------
enum target_platform
getTargetPlatform(void)
{
    return platform_;
}

/*
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
#error "Unknown host platform. Add a new default target platform for the current
host." #endif
    }
}

odb::codegen::TargetTriple targetTriple{*targetTripleArch_,
*targetTriplePlatform_};

// Run AST post-processing (such as type checking, label resolution, etc).
odb::astpost::ProcessGroup post;
post.addProcess(std::make_unique<odb::astpost::ResolveAndCheckTypes>(*cmdIndex));
post.addProcess(std::make_unique<odb::astpost::ResolveLabels>());
if (!post.execute(ast))
{
    return false;
}

// Ensure that the executable extension is .exe if Windows is the target
platform. if (outputIsExecutable_ && targetTriplePlatform_ ==
odb::codegen::TargetTriple::Platform::Windows)
{
    if (outputName.size() < 5 || outputName.substr(outputName.size() - 4, 4) !=
".exe")
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
        odb::Log::codegen(odb::Log::ERROR, "Failed to open file `%s`\n",
outputName.c_str()); return false;
    }

    odb::Log::codegen(odb::Log::INFO, "Creating output file: `%s`\n",
outputName.c_str());
}
std::ostream& outputStream = outputToStdout ? std::cout : *outputFile;
if (!odb::codegen::generateCode(getSDKType(), outputType_, targetTriple,
outputStream, "input.dba", ast, *cmdIndex))
{
    return false;
}

// If we're generating an executable, invoke the linker.
if (outputIsExecutable_)
{
    assert(outputType_ == odb::codegen::OutputType::ObjectFile);

    // Select a linker. By default, we choose the locally embedded LLD linker.
    std::filesystem::path linker =
odb::FileSystem::getPathToSelf().parent_path() / "lld/bin"; switch
(targetTriple.platform) { case odb::codegen::TargetTriple::Platform::Windows:
        linker /= "lld-link";
        break;
    case odb::codegen::TargetTriple::Platform::macOS:
        linker /= "ld64.lld";
        break;
    case odb::codegen::TargetTriple::Platform::Linux:
        linker /= "ld.lld";
        break;
    }

    // Above, we generated an object file and wrote it to `outputName`, even
though it is not an executable
    // yet. Here, we invoke the linker, which takes the above object file
(written to `outputName`), links it, and
    // overwrites the object file with the actual executable.
    if (!odb::codegen::linkExecutable(getSDKType(), getSDKRootDir(), linker,
targetTriple, {outputName}, outputName))
    {
        odb::Log::codegen(odb::Log::ERROR, "Failed to link executable.");
        return false;
    }
}

return true;
}
*/
