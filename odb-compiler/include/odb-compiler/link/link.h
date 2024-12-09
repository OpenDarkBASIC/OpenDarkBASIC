#pragma once

#include "odb-compiler/codegen/target.h"
#include "odb-util/ospath.h"

struct ospathc_list;

ODBCOMPILER_PUBLIC_API int
odb_link(
    struct ospathc_list* obj_files,
    struct ospathc       output_executable,
    enum target_arch     arch,
    enum target_platform platform);
