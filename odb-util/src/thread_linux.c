#define _GNU_SOURCE
#include "odb-util/log.h"
#include "odb-util/thread.h"
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <time.h>

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

