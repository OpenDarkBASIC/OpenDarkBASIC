#pragma once

#include "odb-util/config.h"

struct thread;

/*! Calls func in a new thread. Returns NULL on failure. */
ODBUTIL_PUBLIC_API struct thread*
thread_start(void* (*func)(void*), void* args);

/*! Returns the return value of the thread, or (void*)-1 on failure. */
ODBUTIL_PUBLIC_API void*
thread_join(struct thread* t);

ODBUTIL_PUBLIC_API void
thread_kill(struct thread* t);

