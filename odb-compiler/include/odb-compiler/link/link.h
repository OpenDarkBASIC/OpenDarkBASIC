#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/codegen/target.h"

ODBCOMPILER_PUBLIC_API int
odb_link(
    const char* objs[], int count,
    const char* output_name,
    /*enum odb_sdk_type sdkType,*/
    enum target_arch arch,
    enum target_platform platform);
