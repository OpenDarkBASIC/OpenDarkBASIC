#include "odb-compiler/codegen/target.h"

const char*
target_arch_to_name(enum target_arch arch)
{
    switch (arch)
    {
        case TARGET_i386: return "i386";
        case TARGET_x86_64: return "x86_64";
        case TARGET_AArch64: return "aarch64";
    }
    return "";
}

const char*
target_platform_to_name(enum target_platform platform)
{
    switch (platform)
    {
        case TARGET_WINDOWS: return "windows";
        case TARGET_MACOS: return "macos";
        case TARGET_LINUX: return "linux";
    }
    return "";
}

const char*
target_arch_platform_to_triplet(
    enum target_arch arch, enum target_platform platform)
{
    switch (arch)
    {
        case TARGET_i386:
            switch (platform)
            {
                case TARGET_WINDOWS: return "i386-pc-windows-msvc";
                case TARGET_MACOS: return "i386-apple-darwin";
                case TARGET_LINUX: return "i386-linux-gnu";
            }
            break;
        case TARGET_x86_64:
            switch (platform)
            {
                case TARGET_WINDOWS: return "x86_64-pc-windows-msvc";
                case TARGET_MACOS: return "x86_64-apple-darwin";
                case TARGET_LINUX: return "x86_64-linux-gnu";
            }
            break;
        case TARGET_AArch64:
            switch (platform)
            {
                case TARGET_WINDOWS: return "aarch64-pc-windows-msvc";
                case TARGET_MACOS: return "aarch64-apple-darwin";
                case TARGET_LINUX: return "aarch64-linux-gnu";
            }
            break;
    }
    return "";
}
