#pragma once

#include "odb-util/config.h"
#include "odb-util/ospath.h"

ODBUTIL_PUBLIC_API int
process_run(
    struct ospathc    filepath,
    struct ospathc    working_dir,
    const char* const argv[],
    struct utf8_view  in,
    struct utf8*      out,
    struct utf8*      err,
    int               timeout_ms);
