#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/process.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static char*
argv_to_cstr(const char* const argv[])
{
    char* cstr;
    int   i;

    int len = 0;
    for (i = 0; argv[i] != NULL; ++i)
        len += strlen(argv[i]) + 1;

    cstr = mem_alloc(len + 1);
    if (cstr == NULL)
    {
        log_oom(len + 1, "argv_to_cstr()");
        return NULL;
    }
    *cstr = '\0';

    for (i = 0; argv[i] != NULL; ++i)
    {
        if (i)
            strcat(cstr, " ");
        strcat(cstr, argv[i]);
    }

    return cstr;
}

struct process
{
    HANDLE hProcess;
    HANDLE hThread;
    HANDLE hIn;
    HANDLE hOut;
    HANDLE hErr;
};

struct process*
process_start(
    struct ospathc    filepath,
    struct ospathc    working_dir,
    const char* const argv[],
    uint8_t           flags)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE              hInRead;
    HANDLE              hOutWrite;
    HANDLE              hErrWrite;
    char*               cmdline;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    struct process* process = mem_alloc(sizeof *process);
    if (process == NULL)
        goto alloc_process_failed;
    process->hIn = NULL;
    process->hOut = NULL;
    process->hErr = NULL;

    /* So child process can inherit pipe handles */
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if ((flags & PROCESS_STDIN) && !CreatePipe(&hInRead, &process->hIn, &sa, 0))
    {
        log_util_err("Failed to create stdin pipe: {win32error}\n");
        goto in_pipe_failed;
    }
    if ((flags & PROCESS_STDIN)
        && !SetHandleInformation(process->hIn, HANDLE_FLAG_INHERIT, 0))
    {
        log_util_err("Failed to set stdin handle info: {win32error}\n");
        goto in_pipe_handle_failed;
    }

    if ((flags & PROCESS_STDOUT)
        && !CreatePipe(&process->hOut, &hOutWrite, &sa, 0))
    {
        log_util_err("Failed to create stdout pipe: {win32error}\n");
        goto out_pipe_failed;
    }
    if ((flags & PROCESS_STDOUT)
        && !SetHandleInformation(process->hOut, HANDLE_FLAG_INHERIT, 0))
    {
        log_util_err("Failed to set stdout handle info: {win32error}\n");
        goto out_pipe_handle_failed;
    }

    if ((flags & PROCESS_STDERR)
        && !CreatePipe(&process->hErr, &hErrWrite, &sa, 0))
    {
        log_util_err("Failed to create stderr pipe: {win32error}\n");
        goto err_pipe_failed;
    }
    if ((flags & PROCESS_STDERR)
        && !SetHandleInformation(process->hErr, HANDLE_FLAG_INHERIT, 0))
    {
        log_util_err("Failed to set stderr handle info: {win32error}\n");
        goto err_pipe_handle_failed;
    }

    cmdline = argv_to_cstr(argv);
    if (cmdline == NULL)
        goto cmdline_failed;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    if ((flags & PROCESS_STDIN))
        si.hStdInput = hInRead;
    if ((flags & PROCESS_STDOUT))
        si.hStdOutput = hOutWrite;
    if ((flags & PROCESS_STDERR))
        si.hStdError = hErrWrite;

    ZeroMemory(&pi, sizeof(pi));
    int ret = CreateProcess(
        argv[0],
        cmdline,
        NULL, /* process security */
        NULL, /* primary thread security */
        TRUE, /* Inherit handles */
        NORMAL_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
        NULL, /* Use parents environment */
        working_dir.len > 0 ? ospathc_cstr(working_dir) : NULL,
        &si,
        &pi);
    if (!ret)
    {
        log_util_err(
            "Failed to create process {quote:%s}: {win32error}\n", cmdline);
        goto create_process_failed;
    }
    process->hProcess = pi.hProcess;
    process->hThread = pi.hThread;

    mem_free(cmdline);

    if ((flags & PROCESS_STDIN))
        CloseHandle(hInRead);
    if ((flags & PROCESS_STDOUT))
        CloseHandle(hOutWrite);
    if ((flags & PROCESS_STDERR))
        CloseHandle(hErrWrite);

    return process;

create_process_failed:
    mem_free(cmdline);
cmdline_failed:
err_pipe_handle_failed:
    if ((flags & PROCESS_STDERR))
    {
        CloseHandle(process->hErr);
        CloseHandle(hErrWrite);
    }
err_pipe_failed:
out_pipe_handle_failed:
    if ((flags & PROCESS_STDOUT))
    {
        CloseHandle(process->hOut);
        CloseHandle(hOutWrite);
    }
out_pipe_failed:
in_pipe_handle_failed:
    if ((flags & PROCESS_STDIN))
    {
        CloseHandle(hInRead);
        CloseHandle(process->hIn);
    }
in_pipe_failed:
    mem_free(process);
alloc_process_failed:
    return NULL;
}

int
process_write_stdin(struct process* process, struct utf8_view str)
{
    DWORD dwWritten;
    if (WriteFile(process->hIn, str.data + str.off, str.len, &dwWritten, NULL)
        == FALSE)
    {
        log_util_err("Failed to write to process stdin: {win32error}\n");
        return -1;
    }
    return dwWritten;
}

int
process_read_stdout(struct process* process, char* byte)
{
    DWORD dwBytesRead;
    if (ReadFile(process->hOut, byte, 1, &dwBytesRead, NULL) == FALSE)
    {
        if (GetLastError() == ERROR_BROKEN_PIPE)
            return 0; /* EOF */
        return log_util_err(
            "Failed to read from process stdout: {win32error}\n");
    }
    return dwBytesRead;
}

int
process_read_stderr(struct process* process, char* byte)
{
    DWORD dwBytesRead;
    if (ReadFile(process->hErr, byte, 1, &dwBytesRead, NULL) == FALSE)
    {
        if (GetLastError() == ERROR_BROKEN_PIPE)
            return 0; /* EOF */
        return log_util_err(
            "Failed to read from process stderr: {win32error}\n");
    }
    return dwBytesRead;
}

void
process_terminate(struct process* process)
{
    TerminateProcess(process->hProcess, -1);
}

void
process_kill(struct process* process)
{
    TerminateProcess(process->hProcess, -1);
}

int
process_wait(struct process* process, int timeout_ms)
{
    if (process->hIn)
        CloseHandle(process->hIn);
    process->hIn = NULL;

    if (WaitForSingleObject(
            process->hProcess, timeout_ms > 0 ? timeout_ms : INFINITE)
        != WAIT_OBJECT_0)
    {
        log_util_err("Process did not exit after %dms\n", timeout_ms);
        return -1;
    }

    return 0;
}

int
process_join(struct process* process)
{
    DWORD dwExitCode;
    if (!GetExitCodeProcess(process->hProcess, &dwExitCode))
    {
        log_util_err("Failed to get exit code from process: {win32error}\n");
        dwExitCode = -1;
    }

    if (process->hOut)
        CloseHandle(process->hOut);
    if (process->hErr)
        CloseHandle(process->hErr);

    CloseHandle(process->hThread);
    CloseHandle(process->hProcess);

    mem_free(process);

    return dwExitCode;
}
