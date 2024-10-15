#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "odb-util/mem.h"
#include "odb-util/mutex.h"

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

