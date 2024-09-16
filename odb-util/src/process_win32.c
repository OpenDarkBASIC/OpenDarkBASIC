#include "odb-util/process.h"
#include "odb-util/mem.h"
#include "odb-util/log.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static char*
argv_to_cstr(const char* const argv[])
{
    char* cstr;
    int i;

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
    #define READ 0
    #define WRITE 1
    HANDLE hIn[2];
    HANDLE hOut[2];
    HANDLE hErr[2];
    char* cmdline;
    DWORD exitcode;
    DWORD outlen = 0;
    DWORD errlen = 0;

    /* So child process can inherit pipe handles */
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (in.len && !CreatePipe(&hIn[READ], &hIn[WRITE], &sa, 0))
    {
        log_util_err("Failed to create stdin pipe: {win32error}\n");
        goto in_pipe_failed;
    }
    if (in.len && !SetHandleInformation(hIn[WRITE], HANDLE_FLAG_INHERIT, 0))
    {
        log_util_err("Failed to set stdin handle info: {win32error}\n");
        goto in_pipe_handle_failed;
    }

    if (out && !CreatePipe(&hOut[READ], &hOut[WRITE], &sa, 0))
    {
        log_util_err("Failed to create stdout pipe: {win32error}\n");
        goto out_pipe_failed;
    }
    if (out && !SetHandleInformation(hOut[READ], HANDLE_FLAG_INHERIT, 0))
    {
        log_util_err("Failed to set stdout handle info: {win32error}\n");
        goto out_pipe_handle_failed;
    }

    if (err && !CreatePipe(&hErr[READ], &hErr[WRITE], &sa, 0))
    {
        log_util_err("Failed to create stderr pipe: {win32error}\n");
        goto err_pipe_failed;
    }
    if (err && !SetHandleInformation(hErr[READ], HANDLE_FLAG_INHERIT, 0))
    {
        log_util_err("Failed to set stderr handle info: {win32error}\n");
        goto err_pipe_handle_failed;
    }

    cmdline = argv_to_cstr(argv);
    if (cmdline == NULL)
        goto cmdline_failed;

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    if (in.len)
        si.hStdInput = hIn[READ];
    if (out)
        si.hStdOutput = hOut[WRITE];
    if (err)
        si.hStdError = hErr[WRITE];

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    int ret = CreateProcess(
        argv[0],
        cmdline,
        NULL,  /* process security */
        NULL,  /* primary thread security */
        TRUE,  /* Inherit handles */
        NORMAL_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
        NULL,  /* Use parents environment */
        working_dir.len > 0 ? ospathc_cstr(working_dir) : NULL,
        &si,
        &pi);
    if (!ret)
    {
        log_util_err("Failed to create process {quote:%s}: {win32error}\n", cmdline);
        goto create_process_failed;
    }

    mem_free(cmdline);

    if (in.len)
    {
        DWORD dwWritten;
        CloseHandle(hIn[READ]);
        WriteFile(hIn[WRITE], in.data + in.off, in.len, &dwWritten, NULL);
        CloseHandle(hIn[WRITE]);
    }
    if (out)
        CloseHandle(hOut[WRITE]);
    if (err)
        CloseHandle(hErr[WRITE]);

    do
    {
        if (out)
        {
            utf8_reserve(out, out->len + 4096);
            if (ReadFile(hOut[READ], out->data + out->len, 4096, &outlen, NULL) && outlen > 0)
                out->len += outlen;
        }
        if (err)
        {
            utf8_reserve(err, err->len + 4096);
            if (ReadFile(hErr[READ], err->data + err->len, 4096, &errlen, NULL) && errlen > 0)
                err->len += errlen;
        }
    } while (outlen > 0 || errlen > 0);

    if (out)
        CloseHandle(hOut[READ]);
    if (err)
        CloseHandle(hErr[READ]);

    if (WaitForSingleObject(pi.hProcess, timeout_ms > 0 ? timeout_ms : INFINITE) != WAIT_OBJECT_0)
    {
        TerminateProcess(pi.hProcess, -1);
        log_util_err("Process did not exit after %dms, called TerminateProcess()\n", timeout_ms);
    }
    CloseHandle(pi.hThread);

    if (!GetExitCodeProcess(pi.hProcess, &exitcode))
    {
        CloseHandle(pi.hProcess);
        return log_util_err("Failed to get exit code from process: {win32error}\n");
    
    }
    CloseHandle(pi.hProcess);
    return exitcode;

create_process_failed:
    mem_free(cmdline);
cmdline_failed:
err_pipe_handle_failed:
    if (err)
    {
        CloseHandle(hErr[READ]);
        CloseHandle(hErr[WRITE]);
    }
err_pipe_failed:
out_pipe_handle_failed:
    if (out)
    {
        CloseHandle(hOut[READ]);
        CloseHandle(hOut[WRITE]);
    }
out_pipe_failed:
in_pipe_handle_failed:
    if (in.len)
    {
        CloseHandle(hIn[READ]);
        CloseHandle(hIn[WRITE]);
    }
in_pipe_failed:
    return -1;
}
