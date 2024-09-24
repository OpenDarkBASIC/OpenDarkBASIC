#pragma once

#include "odb-util/config.h"
#include "odb-util/ospath.h"

struct process;

enum process_flags
{
    PROCESS_STDIN = (1 << 0),
    PROCESS_STDOUT = (1 << 1),
    PROCESS_STDERR = (1 << 2),
};

ODBUTIL_PUBLIC_API struct process*
process_start(
    struct ospathc    filepath,
    struct ospathc    working_dir,
    const char* const argv[],
    uint8_t           flags);

ODBUTIL_PUBLIC_API int
process_write_stdin(struct process* process, struct utf8_view str);

ODBUTIL_PUBLIC_API int
process_read_stdout(struct process* process, char* byte);

ODBUTIL_PUBLIC_API int
process_read_stderr(struct process* process, char* byte);

ODBUTIL_PUBLIC_API void
process_terminate(struct process* process);

ODBUTIL_PUBLIC_API void
process_kill(struct process* process);

ODBUTIL_PUBLIC_API int
process_wait(struct process* process, int tiemout_ms);

ODBUTIL_PUBLIC_API int
process_join(struct process* process);

ODBUTIL_PUBLIC_API int
process_run(
    struct ospathc    filepath,
    struct ospathc    working_dir,
    const char* const argv[],
    struct utf8_view  in,
    struct utf8*      out,
    struct utf8*      err,
    int               timeout_ms);
