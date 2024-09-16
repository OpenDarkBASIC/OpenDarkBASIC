#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-util/log.h"
#include "odb-util/mem.h"
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

struct mutex
{
    CRITICAL_SECTION handle;
};

struct mutex*
mutex_create(void)
{
    struct mutex* m = mem_alloc(sizeof *m);
    if (m == NULL)
        return NULL;
    InitializeCriticalSectionAndSpinCount(&m->handle, 0x00000400);
    return m;
}

struct mutex*
mutex_create_recursive(void)
{
    return mutex_create();
}

void
mutex_destroy(struct mutex* m)
{
    DeleteCriticalSection(&m->handle);
    mem_free(m);
}

void
mutex_lock(struct mutex* m)
{
    EnterCriticalSection(&m->handle);
}

int
mutex_trylock(struct mutex* m)
{
    return TryEnterCriticalSection(&m->handle);
}

void
mutex_unlock(struct mutex* m)
{
    LeaveCriticalSection(&m->handle);
}
