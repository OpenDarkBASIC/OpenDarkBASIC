#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/codegen/target.h"
#include "odb-compiler/sdk/sdk_type.h"

ODBCOMPILER_PUBLIC_API int
odb_link(
    const char* objs[], int count,
    const char* output_name,
    enum target_arch arch,
    enum target_platform platform);
