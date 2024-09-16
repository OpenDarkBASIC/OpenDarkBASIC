#include "odb-util/log.h"
#include "odb-util/process.h"
#include "odb-util/utf8.h"
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ  0
#define WRITE 1

static int
communicate(
    int              cpid,
    int*             infd,
    int*             outfd,
    int*             errfd,
    struct utf8_view in,
    struct utf8*     out,
    struct utf8*     err)
{
    int wstatus;
    int rc;
    int outlen = 0;
    int errlen = 0;

    if (in.len)
    {
        close(infd[READ]);
        write(infd[WRITE], in.data + in.off, in.len);
        close(infd[WRITE]);
    }
    if (out)
    {
        close(outfd[WRITE]);
        out->len = 0;
    }
    if (err)
    {
        close(errfd[WRITE]);
        err->len = 0;
    }
    do
    {
        if (out)
        {
            utf8_reserve(out, out->len + 4096);
            outlen = read(outfd[READ], out->data + out->len, 4096);
            if (outlen > 0)
                out->len += outlen;
        }
        if (err)
        {
            utf8_reserve(err, err->len + 4096);
            errlen = read(errfd[READ], err->data + err->len, 4096);
            if (errlen > 0)
                err->len += errlen;
        }
    } while (outlen > 0 || errlen > 0);
    if (out)
        close(outfd[READ]);
    if (err)
        close(errfd[READ]);

    do
    {
        rc = waitpid(cpid, &wstatus, 0);
    } while (rc != cpid || errno == ECHILD);

    if (WIFEXITED(wstatus))
        return WEXITSTATUS(wstatus);
    return -1;
}

int
process_run(
    struct ospathc    filepath,
    const char* const argv[],
    struct utf8_view  in,
    struct utf8*      out,
    struct utf8*      err,
    int               timeout_ms)
{
    int infd[2];
    int outfd[2];
    int errfd[2];
    int cpid;

    if (in.len && pipe(infd) != 0)
    {
        log_util_err("Failed to create stdin pipe: %s\n", strerror(errno));
        goto in_pipe_failed;
    }
    if (out && pipe(outfd) != 0)
    {
        log_util_err("Failed to create stdout pipe: %s\n", strerror(errno));
        goto out_pipe_failed;
    }
    if (err && pipe(errfd) != 0)
    {
        log_util_err("Failed to create stderr pipe: %s\n", strerror(errno));
        goto err_pipe_failed;
    }

    cpid = fork();
    if (cpid == -1)
    {
        log_util_err("fork() failed: %s\n", strerror(errno));
        goto fork_failed;
    }

    if (cpid == 0)
    {
        pid_t worker_pid, timeout_pid, exited_pid;
        int   wstatus;
        if (in.len)
        {
            close(infd[WRITE]);
            dup2(infd[READ], STDIN_FILENO);
            close(infd[READ]);
        }
        if (out)
        {
            close(outfd[READ]);
            dup2(outfd[WRITE], STDOUT_FILENO);
            close(outfd[WRITE]);
        }
        if (err)
        {
            close(errfd[READ]);
            dup2(errfd[WRITE], STDERR_FILENO);
            close(errfd[WRITE]);
        }

        timeout_pid = -1;
        if (timeout_ms > 0)
        {
            timeout_pid = fork();
            if (timeout_pid == -1)
            {
                log_util_err("fork() failed: %s\n", strerror(errno));
                _exit(-1);
            }
            if (timeout_pid == 0)
            {
                usleep(timeout_ms * 1000);
                log_util_err("Process timed out\n");
                _exit(0);
            }
        }

        worker_pid = fork();
        if (worker_pid == -1)
        {
            log_util_err("fork() failed: %s\n", strerror(errno));
            _exit(-1);
        }
        if (worker_pid == 0)
        {
            execvp(ospathc_cstr(filepath), (char* const*)argv);
            log_util_err(
                "Failed to exec %s: %s\n",
                ospathc_cstr(filepath),
                strerror(errno));
            _exit(-1);
        }

        exited_pid = wait(&wstatus);
        if (exited_pid == worker_pid && timeout_ms > 0)
            kill(timeout_pid, SIGKILL);
        if (exited_pid == timeout_pid)
            kill(worker_pid, SIGKILL);

        wait(NULL);
        if (WIFEXITED(wstatus))
            _exit(WEXITSTATUS(wstatus));
        _exit(-1);
    }

    return communicate(cpid, infd, outfd, errfd, in, out, err);

fork_failed:
    if (err)
    {
        close(errfd[READ]);
        close(errfd[WRITE]);
    }
err_pipe_failed:
    if (out)
    {
        close(outfd[READ]);
        close(outfd[WRITE]);
    }
out_pipe_failed:
    if (in.len)
    {
        close(infd[READ]);
        close(infd[WRITE]);
    }
in_pipe_failed:
    return -1;
}
