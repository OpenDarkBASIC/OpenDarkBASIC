#pragma once

#include "odb-compiler/config.h"

/* clang-format off */
#define TARGET_ARCH_LIST      \
    X(i386)                   \
    X(x86_64)                 \
    X(AArch64)

#define TARGET_PLATFORM_LIST  \
    X(WINDOWS)                \
    X(MACOS)                  \
    X(LINUX)
/* clang-format on */

enum target_arch
{
#define X(name) TARGET_##name,
    TARGET_ARCH_LIST
#undef X
};

enum target_platform
{
#define X(name) TARGET_##name,
    TARGET_PLATFORM_LIST
#undef X
};

struct ast;
struct cmd_list;
struct ir_module;

ODBCOMPILER_PUBLIC_API const char*
target_arch_to_name(enum target_arch arch);

ODBCOMPILER_PUBLIC_API const char*
target_platform_to_name(enum target_platform platform);
