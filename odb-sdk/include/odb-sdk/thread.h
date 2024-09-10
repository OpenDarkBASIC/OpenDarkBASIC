#pragma once

#include "odb-sdk/config.h"

struct thread;
struct mutex;

/*! Calls func in a new thread. Returns NULL on failure. */
ODBSDK_PUBLIC_API struct thread*
thread_start(void* (*func)(void*), void* args);

/*! Returns the return value of the thread, or (void*)-1 on failure. */
ODBSDK_PUBLIC_API void*
thread_join(struct thread* t);

ODBSDK_PUBLIC_API void
thread_kill(struct thread* t);

ODBSDK_PUBLIC_API struct mutex*
mutex_create(void);

ODBSDK_PUBLIC_API struct mutex*
mutex_create_recursive(void);

ODBSDK_PUBLIC_API void
mutex_destroy(struct mutex* m);

ODBSDK_PUBLIC_API void
mutex_lock(struct mutex* m);

/*!
 * \brief Attempts to lock a mutex.
 * \param[in] m Mutex
 * \return Returns non-zero if the lock was acquired. Zero if the lock was not
 * acquired.
 */
ODBSDK_PUBLIC_API int
mutex_trylock(struct mutex* m);

ODBSDK_PUBLIC_API void
mutex_unlock(struct mutex* m);
