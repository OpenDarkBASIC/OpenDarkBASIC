#define _GNU_SOURCE
#include "odb-util/mem.h"
#include "odb-util/mutex.h"
#include <pthread.h>
#include <string.h>
#include <time.h>

struct mutex
{
    pthread_mutex_t handle;
};

struct mutex*
mutex_create(void)
{
    pthread_mutexattr_t attr;
    struct mutex*       m;
    int                 rc;

    m = mem_alloc(sizeof(*m));
    if (m == NULL)
        goto alloc_mutex_failed;

    pthread_mutexattr_init(&attr);
    rc = pthread_mutex_init(&m->handle, &attr);
    pthread_mutexattr_init(&attr);
    if (rc != 0)
        goto init_mutex_failed;

    return m;

init_mutex_failed:
    mem_free(m);
alloc_mutex_failed:
    return NULL;
}

struct mutex*
mutex_create_recursive(void)
{
    pthread_mutexattr_t attr;
    struct mutex*       m;
    int                 rc;

    m = mem_alloc(sizeof(*m));
    if (m == NULL)
        goto alloc_mutex_failed;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    rc = pthread_mutex_init(&m->handle, &attr);
    pthread_mutexattr_destroy(&attr);
    if (rc != 0)
        goto init_mutex_failed;

    return m;

init_mutex_failed:
    mem_free(m);
alloc_mutex_failed:
    return NULL;
}

void
mutex_destroy(struct mutex* m)
{
    pthread_mutex_destroy(&m->handle);
    mem_free(m);
}

void
mutex_lock(struct mutex* m)
{
    pthread_mutex_lock(&m->handle);
}

int
mutex_trylock(struct mutex* m)
{
    return pthread_mutex_trylock(&m->handle) == 0;
}

void
mutex_unlock(struct mutex* m)
{
    pthread_mutex_unlock(&m->handle);
}

