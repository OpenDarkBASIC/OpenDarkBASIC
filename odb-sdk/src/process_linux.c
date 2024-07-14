#include "odb-sdk/log.h"
#include "odb-sdk/process.h"
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

int
process_run(
    struct ospathc    filepath,
    const char* const argv[],
    struct utf8_view  in,
    struct utf8*      out,
    struct utf8*      err)
{
    int infd[2];
    int outfd[2];
    int errfd[2];
    int cpid;

    if (in.len && pipe(infd) != 0)
    {
        log_sdk_err("Failed to create stdin pipe: %s\n", strerror(errno));
        goto in_pipe_failed;
    }
    if (out && pipe(outfd) != 0)
    {
        log_sdk_err("Failed to create stdout pipe: %s\n", strerror(errno));
        goto out_pipe_failed;
    }
    if (err && pipe(errfd) != 0)
    {
        log_sdk_err("Failed to create stderr pipe: %s\n", strerror(errno));
        goto err_pipe_failed;
    }

    cpid = fork();
    if (cpid == -1)
        goto fork_failed;

    if (cpid == 0)
    {
        if (in.len)
        {
            close(infd[1]);
            dup2(infd[0], STDIN_FILENO);
            close(infd[0]);
        }
        if (out)
        {
            close(outfd[0]);
            dup2(outfd[1], STDOUT_FILENO);
            close(outfd[1]);
        }
        if (err)
        {
            close(errfd[0]);
            dup2(errfd[1], STDERR_FILENO);
            close(errfd[1]);
        }

        execvp(ospathc_cstr(filepath), (char* const*)argv);
        log_sdk_err(
            "Failed to exec %s: %s\n", ospathc_cstr(filepath), strerror(errno));
        _exit(-1);
    }
    else
    {
        int wstatus;
        int rc;
        int outlen = 0;
        int errlen = 0;

        if (in.len)
        {
            close(infd[0]);
            write(infd[1], in.data + in.off, in.len);
            close(infd[1]);
        }
        if (out)
        {
            close(outfd[1]);
            out->len = 0;
        }
        if (err)
        {
            close(errfd[1]);
            err->len = 0;
        }
        do
        {
            if (out)
            {
                utf8_resize(out, out->len + 4096);
                outlen = read(outfd[0], out->data + out->len, 4096);
                if (outlen > 0)
                    out->len += outlen;
            }
            if (err)
            {
                utf8_resize(err, err->len + 4096);
                errlen = read(errfd[0], err->data + err->len, 4096);
                if (errlen > 0)
                    err->len += errlen;
            }
        } while (outlen > 0 || errlen > 0);
        if (out)
            close(outfd[0]);
        if (err)
            close(errfd[0]);

        do
        {
            rc = waitpid(cpid, &wstatus, 0);
        } while (rc != cpid || errno == ECHILD);

        if (WIFEXITED(wstatus))
            return WEXITSTATUS(wstatus);
        return -1;
    }

fork_failed:
    if (err)
    {
        close(errfd[0]);
        close(errfd[1]);
    }
err_pipe_failed:
    if (out)
    {
        close(outfd[0]);
        close(outfd[1]);
    }
out_pipe_failed:
    if (in.len)
    {
        close(infd[0]);
        close(infd[1]);
    }
in_pipe_failed:
    return -1;
}
