#pragma once

#include "odb-sdk/config.h"

struct thread
{
    void* handle;
};
struct mutex
{
    void* handle;
};

ODBSDK_PUBLIC_API int
thread_start(struct thread* t, void* (*func)(void*), void* args);

ODBSDK_PUBLIC_API int
thread_join(struct thread t, int timeout_ms);

ODBSDK_PUBLIC_API void
thread_kill(struct thread t);

ODBSDK_PUBLIC_API void
mutex_init(struct mutex* m);

ODBSDK_PUBLIC_API void
mutex_init_recursive(struct mutex* m);

ODBSDK_PUBLIC_API void
mutex_deinit(struct mutex m);

ODBSDK_PUBLIC_API void
mutex_lock(struct mutex m);

/*!
 * \brief Attempts to lock a mutex.
 * \param[in] m Mutex
 * \return Returns non-zero if the lock was acquired. Zero if the lock was not
 * acquired.
 */
ODBSDK_PUBLIC_API int
mutex_trylock(struct mutex m);

ODBSDK_PUBLIC_API void
mutex_unlock(struct mutex m);
