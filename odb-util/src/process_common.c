#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/process.h"
#include "odb-util/thread.h"

struct read_thread_ctx
{
    struct process* process;
    struct utf8*    buf;
    int (*read)(struct process*, char*);
};

static void*
read_thread(void* param)
{
    char                    byte;
    struct read_thread_ctx* ctx = param;

    mem_init();
    mem_acquire(ctx->buf->data, ctx->buf->len);

    while (ctx->read(ctx->process, &byte) > 0)
    {
        if (utf8_reserve(ctx->buf, ctx->buf->len + 1) != 0)
        {
            mem_release(ctx->buf->data);
            mem_deinit();
            return (void*)-1;
        }
        ctx->buf->data[ctx->buf->len++] = byte;
    }

    mem_release(ctx->buf->data);
    mem_deinit();
    return (void*)0;
}

int
process_run(
    struct ospathc    filepath,
    struct ospathc    working_dir,
    const char* const argv[],
    struct utf8_view  in,
    struct utf8*      out,
    struct utf8*      err,
    int               timeout_ms)
{
    int                    exit_code, out_result, err_result;
    struct read_thread_ctx out_ctx, err_ctx;
    struct thread*         tout = NULL;
    struct thread*         terr = NULL;

    struct process* p = process_start(
        filepath,
        working_dir,
        argv,
        (in.len ? PROCESS_STDIN : 0) | (out ? PROCESS_STDOUT : 0)
            | (err ? PROCESS_STDERR : 0));
    if (p == NULL)
        return -1;

    if (in.len)
        if (process_write_stdin(p, in) <= 0)
        {
            process_kill(p);
            return -1;
        }

    if (out)
    {
        out_ctx.process = p;
        out_ctx.buf = out;
        out_ctx.read = process_read_stdout;
        mem_release(out->data);
        tout = thread_start(read_thread, &out_ctx);
    }
    if (err)
    {
        err_ctx.process = p;
        err_ctx.buf = err;
        err_ctx.read = process_read_stderr;
        mem_release(err->data);
        terr = thread_start(read_thread, &err_ctx);
    }

    if (process_wait(p, timeout_ms) != 0)
    {
        if (timeout_ms)
            log_warn(
                "",
                "Process did not exit after %dms, calling terminate()\n",
                timeout_ms);
        else
            log_warn("", "Process did not exit cleanly, calling terminate()\n");

        process_terminate(p);
        if (process_wait(p, 500) != 0)
        {
            log_warn(
                "", "Process did not terminate after 500ms, calling kill()\n");
            process_kill(p);
            process_wait(p, 0);
        }
    }

    out_result = tout ? (int)(intptr_t)thread_join(tout) : 0;
    err_result = terr ? (int)(intptr_t)thread_join(terr) : 0;
    exit_code = process_join(p);

    if (out)
        mem_acquire(out->data, out->len);
    if (err)
        mem_acquire(err->data, err->len);

    if (out_result == 0 && err_result == 0)
        return exit_code;

    return -1;
}
