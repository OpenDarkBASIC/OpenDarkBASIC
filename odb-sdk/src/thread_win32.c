#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/thread.h"

int
thread_start(struct thread* t, void* (*func)(const void*), const void* args)
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
        log_err("Failed to create thread\n");
        return -1;
    }

    t->handle = (void*)hThread;
    return 0;
}

int
thread_join(struct thread t, int timeout_ms)
{
    HANDLE hThread = (HANDLE)t.handle;
    switch (WaitForSingleObject(hThread, timeout_ms == 0 ? INFINITE : (DWORD)timeout_ms))
    {
    case WAIT_FAILED:
        log_err("WaitForSingleObject failed in thread_join(): %d\n", GetLastError());
        /* fallthrough */
    case WAIT_ABANDONED:
    case WAIT_TIMEOUT:
        return -1;

    default:
        break;
    }
    CloseHandle(hThread);
    return 0;
}

void
thread_kill(struct thread t)
{
    HANDLE hThread = (HANDLE)t.handle;
    if (TerminateThread(hThread, (DWORD)-1) == FALSE)
        log_err("Failed to TerminateThread: %d\n", GetLastError());
    CloseHandle(hThread);
}

void
mutex_init(struct mutex* m)
{
    m->handle = mem_alloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSectionAndSpinCount(m->handle, 0x00000400);
}

void
mutex_init_recursive(struct mutex* m)
{
    m->handle = mem_alloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSectionAndSpinCount(m->handle, 0x00000400);
}

void
mutex_deinit(struct mutex m)
{
    DeleteCriticalSection(m.handle);
    mem_free(m.handle);
}

void
mutex_lock(struct mutex m)
{
    EnterCriticalSection(m.handle);
}

int
mutex_trylock(struct mutex m)
{
    return TryEnterCriticalSection(m.handle);
}

void
mutex_unlock(struct mutex m)
{
    LeaveCriticalSection(m.handle);
}
