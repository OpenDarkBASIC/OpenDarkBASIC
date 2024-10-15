#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-util/log.h"
#include "odb-util/thread.h"

struct thread*
thread_start(void* (*func)(const void*), const void* args)
{
    HANDLE hThread = CreateThread(
        NULL,  /* Security attributes*/
        0,     /* Initial stack size */
        (LPTHREAD_START_ROUTINE)func,
        (void*)args,
        0,     /* Run thread immediately */
        NULL); /* tid */
    if (hThread == NULL)
    {
        log_util_err("Failed to create thread: {win32error}\n");
        return NULL;
    }

    return (struct thread*)hThread;
}

void*
thread_join(struct thread* t)
{
    DWORD ret;
    HANDLE hThread = (HANDLE)t;
    if (WaitForSingleObject(hThread, INFINITE) != 0)
    {
        log_util_err("WaitForSingleObject failed in thread_join(): {win32error}\n");
        return (void*)-1;
    }

    GetExitCodeThread(hThread, &ret);
    CloseHandle(hThread);
    return (void*)(intptr_t)ret;
}

void
thread_kill(struct thread* t)
{
    HANDLE hThread = (HANDLE)t;
    if (TerminateThread(hThread, (DWORD)-1) == FALSE)
        log_util_err("Failed to TerminateThread: {win32error}\n");
    CloseHandle(hThread);
}

