#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/process.h"
#include "odb-util/utf8.h"
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ  0
#define WRITE 1

struct process
{
    pid_t pid;
    int   stdin_fd;
    int   stdout_fd;
    int   stderr_fd;
    int   exit_code;
};

struct process*
process_start(
    struct ospathc    filepath,
    struct ospathc    working_dir,
    const char* const argv[],
    uint8_t           flags)
{
    int infd[2], outfd[2], errfd[2];
    int cpid;

    struct process* process = mem_alloc(sizeof *process);
    if (process == NULL)
        goto alloc_process_failed;
    process->stdin_fd = -1;
    process->stdout_fd = -1;
    process->stderr_fd = -1;
    process->exit_code = -1;

    if ((flags & PROCESS_STDIN) && pipe(infd) != 0)
    {
        log_util_err("Failed to create stdin pipe: %s\n", strerror(errno));
        goto in_pipe_failed;
    }
    if ((flags & PROCESS_STDOUT) && pipe(outfd) != 0)
    {
        log_util_err("Failed to create stdout pipe: %s\n", strerror(errno));
        goto out_pipe_failed;
    }
    if ((flags & PROCESS_STDERR) && pipe(errfd) != 0)
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
        char* abs_filepath = realpath(ospathc_cstr(filepath), NULL);
        if (abs_filepath == NULL)
        {
            log_util_err(
                "Failed to resolve absolute path for %s: %s\n",
                ospathc_cstr(filepath),
                strerror(errno));
            _exit(-1);
        }

        if (flags & PROCESS_STDIN)
        {
            close(infd[WRITE]);
            dup2(infd[READ], STDIN_FILENO);
            close(infd[READ]);
        }
        if (flags & PROCESS_STDOUT)
        {
            close(outfd[READ]);
            dup2(outfd[WRITE], STDOUT_FILENO);
            close(outfd[WRITE]);
        }
        if (flags & PROCESS_STDERR)
        {
            close(errfd[READ]);
            dup2(errfd[WRITE], STDERR_FILENO);
            close(errfd[WRITE]);
        }

        if (working_dir.len > 0 && chdir(ospathc_cstr(working_dir)) != 0)
        {
            log_util_err(
                "Failed to change working directory to {quote:%s}: %s\n",
                ospathc_cstr(working_dir),
                strerror(errno));
            _exit(-1);
        }

        execvp(abs_filepath, (char* const*)argv);
        log_util_err(
            "Failed to exec %s: %s\n", ospathc_cstr(filepath), strerror(errno));
        _exit(-1);
    }

    process->pid = cpid;
    if (flags & PROCESS_STDIN)
    {
        close(infd[READ]);
        process->stdin_fd = infd[WRITE];
    }
    if (flags & PROCESS_STDOUT)
    {
        close(outfd[WRITE]);
        process->stdout_fd = outfd[READ];
    }
    if (flags & PROCESS_STDERR)
    {
        close(errfd[WRITE]);
        process->stderr_fd = errfd[READ];
    }

    return process;

fork_failed:
    if (flags & PROCESS_STDERR)
    {
        close(errfd[READ]);
        close(errfd[WRITE]);
    }
err_pipe_failed:
    if (flags & PROCESS_STDOUT)
    {
        close(outfd[READ]);
        close(outfd[WRITE]);
    }
out_pipe_failed:
    if (flags & PROCESS_STDIN)
    {
        close(infd[READ]);
        close(infd[WRITE]);
    }
in_pipe_failed:
    mem_free(process);
alloc_process_failed:
    return NULL;
}

int
process_write_stdin(struct process* process, struct utf8_view str)
{
    int rc = write(process->stdin_fd, str.data + str.off, str.len);
    if (rc == -1)
        log_util_err("Failed to write to process stdin: %s\n", strerror(errno));
    return rc;
}

int
process_read_stdout(struct process* process, char* byte)
{
    int rc = read(process->stdout_fd, byte, 1);
    if (rc < 0)
        return log_util_err(
            "Failed to read from process stdout: %s\n", strerror(errno));
    return rc;
}

int
process_read_stderr(struct process* process, char* byte)
{
    int rc = read(process->stderr_fd, byte, 1);
    if (rc < 0)
        return log_util_err(
            "Failed to read from process stderr: %s\n", strerror(errno));
    return rc;
}

void
process_terminate(struct process* process)
{
    kill(process->pid, SIGTERM);
}

void
process_kill(struct process* process)
{
    kill(process->pid, SIGKILL);
}

int
process_wait(struct process* process, int timeout_ms)
{
    pid_t timeout_pid = -1;

    if (process->stdin_fd != -1)
        close(process->stdin_fd);
    process->stdin_fd = -1;

    if (timeout_ms > 0)
    {
        timeout_pid = fork();
        if (timeout_pid == -1)
            return log_util_err(
                "fork() failed for timeout process: %s\n", strerror(errno));
        if (timeout_pid == 0)
        {
            usleep(timeout_ms * 1000);
            log_util_err("Process timed out\n");
            _exit(0);
        }
    }

    while (1)
    {
        int   wstatus;
        pid_t exited_pid = wait(&wstatus);
        if (exited_pid == -1)
        {
            if (errno == ECHILD)
                return 0; /* No child processes exist */
            return -1;
        }
        if (exited_pid == process->pid)
        {
            if (timeout_ms > 0)
                kill(timeout_pid, SIGKILL);
            if (WIFEXITED(wstatus))
                process->exit_code = WEXITSTATUS(wstatus);
            return 0;
        }
        if (exited_pid == timeout_pid)
        {
            kill(process->pid, SIGKILL);
            return -1;
        }
    }
}

int
process_join(struct process* process)
{
    int exit_code = process->exit_code;

    if (process->stdout_fd != -1)
        close(process->stdout_fd);
    if (process->stderr_fd != -1)
        close(process->stderr_fd);

    mem_free(process);
    return exit_code;
}
