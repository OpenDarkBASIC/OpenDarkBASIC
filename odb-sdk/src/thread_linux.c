#define _GNU_SOURCE
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "vh/log.h"
#include "vh/mem.h"
#include "vh/thread.h"

int
thread_start(struct thread* t, void* (*func)(void*), void* args)
{
    int rc;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if ((rc = pthread_create((pthread_t*)&t->handle, &attr, (void*(*)(void*))func, (void*)args)) != 0)
    {
        pthread_attr_destroy(&attr);
        log_err("Failed to create thread: %s\n", strerror(rc));
        return -1;
    }

    pthread_attr_destroy(&attr);
    return 0;
}

int
thread_join(struct thread t, int timeout_ms)
{
    void* ret;
    struct timespec ts;
    struct timespec off;

    clock_gettime(CLOCK_REALTIME, &ts);
    off.tv_sec = timeout_ms / 1000;
    off.tv_nsec = (timeout_ms - ts.tv_sec * 1000) * 1000000;
    ts.tv_nsec += off.tv_nsec;
    while (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }
    ts.tv_sec += off.tv_sec;

    if (pthread_timedjoin_np((pthread_t)t.handle, &ret, &ts) == 0)
        return 0;
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
