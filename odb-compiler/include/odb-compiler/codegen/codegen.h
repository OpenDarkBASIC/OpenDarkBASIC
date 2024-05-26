#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/sdk.h"

enum odb_codegen_output_type
{
    ODB_CODEGEN_LLVMIR,
    ODB_CODEGEN_LLVMBitcode,
    ODB_CODEGEN_ObjectFile
};

enum odb_codegen_arch
{
    ODB_CODEGEN_i386,
    ODB_CODEGEN_x86_64,
    ODB_CODEGEN_AArch64
};

enum odb_codegen_platform
{
    ODB_CODEGEN_WINDOWS,
    ODB_CODEGEN_MACOS,
    ODB_CODEGEN_LINUX
};

struct ast;
struct cmd_list;

ODBCOMPILER_PUBLIC_API int
odb_codegen(
    struct ast* program,
    const char* output_name,
    const char* module_name,
    enum sdk_type sdkType,
    enum odb_codegen_output_type output_type,
    enum odb_codegen_arch        arch,
    enum odb_codegen_platform    platform,
    const struct cmd_list*       cmds,
    const char*                  source_filename,
    struct db_source             source);

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
