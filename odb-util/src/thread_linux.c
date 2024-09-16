#define _GNU_SOURCE
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/thread.h"
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>

struct mutex
{
    pthread_mutex_t handle;
};

struct thread*
thread_start(void* (*func)(void*), void* args)
{
    int            rc;
    pthread_attr_t attr;
    pthread_t      t;

    pthread_attr_init(&attr);
    rc = pthread_create(&t, &attr, (void* (*)(void*))func, (void*)args);
    pthread_attr_destroy(&attr);
    if (rc != 0)
    {
        log_util_err("Failed to create thread: %s\n", strerror(rc));
        return NULL;
    }

    return (struct thread*)t;
}

void*
thread_join(struct thread* t)
{
    void* ret;
    int   rc;

    rc = pthread_join((pthread_t)t, &ret);
    if (rc == 0)
        return ret;

    log_util_err("Failed to join thread: %s\n", strerror(rc));
    return (void*)-1;
}

void
thread_kill(struct thread* t)
{
    pthread_kill((pthread_t)t, SIGKILL);
}

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
