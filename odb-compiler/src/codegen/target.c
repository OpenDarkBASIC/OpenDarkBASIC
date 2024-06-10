#include "odb-compiler/codegen/target.h"

const char*
target_arch_to_name(enum target_arch arch)
{
    switch (arch)
    {
        case TARGET_i386: return "i386";
        case TARGET_x86_64: return "x86_64";
        case TARGET_AArch64: return "AArch64";
    }
    return "";
}

const char*
target_platform_to_name(enum target_platform platform)
{
    switch (platform)
    {
        case TARGET_WINDOWS: return "Windows";
        case TARGET_MACOS: return "MacOS";
        case TARGET_LINUX: return "Linux";
    }
    return "";
}
