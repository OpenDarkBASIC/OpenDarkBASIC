#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/sdk.h"

struct ast;
struct cmd_list;
struct ir_module;

ODBCOMPILER_PUBLIC_API struct ir_module*
ir_alloc(const char* module_name);

ODBCOMPILER_PUBLIC_API void
ir_free(struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_translate_ast(
    struct ir_module*      ir,
    struct ast*            program,
    enum sdk_type          sdkType,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source);

ODBCOMPILER_PUBLIC_API int
ir_create_runtime(
    struct ir_module*    ir,
    enum sdk_type        sdk_type,
    enum target_arch     arch,
    enum target_platform platform);

ODBCOMPILER_PUBLIC_API int
ir_optimize(struct ir_module* ir);

ODBCOMPILER_PUBLIC_API int
ir_compile(
    struct ir_module*    mod,
    const char*          output_filepath,
    enum target_arch     arch,
    enum target_platform platform);

/*ODBCOMPILER_PUBLIC_API bool linkExecutable(SDKType sdkType, const
   std::filesystem::path& sdkRootDir, const std::filesystem::path& linker,
   TargetTriple targetTriple, std::vector<std::string> inputFilenames,
   std::string& outputFilename);*/

/*
struct TargetTriple
{
    enum class Arch
    {
        i386,
        x86_64,
        AArch64,
    };

    enum class Platform
    {
        Windows,
        macOS,
        Linux,
    };

    Arch arch;
    Platform platform;

    std::string getLLVMTargetTriple() const
    {
        // Examples:
        //   i386-pc-windows-msvc
        //   x86_64-pc-linux-gnu
        std::string target_triple;
        switch (arch)
        {
        case TargetTriple::Arch::i386:
            target_triple += "i386";
            break;
        case TargetTriple::Arch::x86_64:
            target_triple += "x86_64";
            break;
        case TargetTriple::Arch::AArch64:
            target_triple += "aarch64";
            break;
        }
        target_triple += "-";
        switch (platform)
        {
        case TargetTriple::Platform::Windows:
            target_triple += "pc-windows-msvc";
            break;
        case TargetTriple::Platform::macOS:
            target_triple += "apple-darwin";
            break;
        case TargetTriple::Platform::Linux:
            target_triple += "pc-linux-gnu";
            break;
        }
        return target_triple;
    }
};
*/
