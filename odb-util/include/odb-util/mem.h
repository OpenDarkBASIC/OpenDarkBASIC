#pragma once

#include "odb-util/config.h"
#include <stdint.h>

typedef uint32_t mem_size;
typedef int32_t mem_idx;

#if !defined(ODBUTIL_MEM_DEBUGGING)
#   include <stdlib.h>
#   define mem_init() (0)
#   define mem_deinit() (0)
#   define mem_alloc     malloc
#   define mem_free      free
#   define mem_realloc   realloc
#   define mem_track_allocation(p)
#   define mem_track_deallocation(p)
#   define mem_acquire(p, s)
#   define mem_release(p)
#else

/*!
 * @brief Initializes memory tracking. This is called by odbutil_init().
 *
 * In release mode this does nothing. In debug mode it will initialize
 * memory reports and backtraces, if enabled.
 */
ODBUTIL_PUBLIC_API int
mem_init(void);

/*!
 * @brief De-initializes memory tracking. This is called from odbutil_deinit().
 *
 * In release mode this does nothing. In debug mode this will output the memory
 * report and print backtraces, if enabled.
 * @return Returns the number of memory leaks.
 */
ODBUTIL_PUBLIC_API mem_size
mem_deinit(void);

/*!
 * @brief Does the same thing as a normal call to malloc(), but does some
 * additional work to monitor and track down memory leaks.
 */
ODBUTIL_PUBLIC_API void*
mem_alloc(mem_size size);

/*!
 * @brief Does the same thing as a normal call to realloc(), but does some
 * additional work to monitor and track down memory leaks.
 */
ODBUTIL_PUBLIC_API void*
mem_realloc(void* ptr, mem_size new_size);

/*!
 * @brief Does the same thing as a normal call to fee(), but does some
 * additional work to monitor and track down memory leaks.
 */
ODBUTIL_PUBLIC_API void
mem_free(void*);

ODBUTIL_PUBLIC_API void
mem_track_allocation(void* p);

ODBUTIL_PUBLIC_API void
mem_track_deallocation(void* p);

ODBUTIL_PUBLIC_API void
mem_acquire(void* p, mem_size size);

ODBUTIL_PUBLIC_API mem_size
mem_release(void* p);

#endif

