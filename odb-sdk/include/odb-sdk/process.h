#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/ospath.h"

ODBSDK_PUBLIC_API int
process_run(
    struct ospathc    filepath,
    const char* const argv[],
    struct utf8_view  in,
    struct utf8*      out,
    struct utf8*      err,
    int               timeout_ms);
