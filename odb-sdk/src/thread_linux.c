#define _GNU_SOURCE
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/thread.h"
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int
thread_start(struct thread* t, void* (*func)(void*), void* args)
{
    int            rc;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if ((rc = pthread_create(
             (pthread_t*)&t->handle,
             &attr,
             (void* (*)(void*))func,
             (void*)args))
        != 0)
    {
        pthread_attr_destroy(&attr);
        log_sdk_err("Failed to create thread: %s\n", strerror(rc));
        return -1;
    }

    pthread_attr_destroy(&attr);
    return 0;
}

int
thread_join(struct thread t, void** ret)
{
    int rc = pthread_join((pthread_t)t.handle, ret);
    if (rc == 0)
        return 0;

    log_sdk_err("Failed to join thread: %s\n", strerror(rc));
    return -1;
}

void
thread_kill(struct thread t)
{
    pthread_kill((pthread_t)t.handle, SIGKILL);
}

void
mutex_init(struct mutex* m)
{
    pthread_mutexattr_t attr;
    m->handle = mem_alloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(m->handle, &attr);
    pthread_mutexattr_destroy(&attr);
}

void
mutex_init_recursive(struct mutex* m)
{
    pthread_mutexattr_t attr;
    m->handle = mem_alloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(m->handle, &attr);
    pthread_mutexattr_destroy(&attr);
}

void
mutex_deinit(struct mutex m)
{
    pthread_mutex_destroy(m.handle);
    mem_free(m.handle);
}

void
mutex_lock(struct mutex m)
{
    pthread_mutex_lock(m.handle);
}

int
mutex_trylock(struct mutex m)
{
    return pthread_mutex_trylock(m.handle) == 0;
}

void
mutex_unlock(struct mutex m)
{
    pthread_mutex_unlock(m.handle);
}
