#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/codegen/codegen.h"

ODBCOMPILER_PUBLIC_API int
odb_link(
    const char* objs[], int count,
    const char* output_name,
    /*enum odb_sdk_type sdkType,*/
    enum odb_codegen_arch arch,
    enum odb_codegen_platform platform);
