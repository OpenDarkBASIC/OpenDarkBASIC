#pragma once

#include "odb-sdk/config.h"
#include <stdint.h>

#if defined(ODBSDK_PLATFORM_WINDOWS)
#   include <malloc.h>
static inline int mem_allocated_size(void* p) { return (int)_msize(p); }
#elif defined(ODBSDK_PLATFORM_DARWIN)
#   include <malloc/malloc.h>
#   define mem_allocated_size  malloc_size
#elif defined(ODBSDK_PLATFORM_LINUX)
#   include <malloc.h>
#   define mem_allocated_size  malloc_usable_size
#else
#   error "Unknown Platform"
#endif

typedef uint32_t mem_size;
typedef int32_t mem_idx;

#if !defined(ODBSDK_MEM_DEBUGGING)
#   include <stdlib.h>
#   define mem_init() (0)
#   define mem_deinit()
#   define mem_alloc     malloc
#   define mem_free      free
#   define mem_realloc   realloc
#   define mem_track_allocation(p)
#   define mem_track_deallocation(p)
#else

/*!
 * @brief Initializes memory tracking. This is called by odbsdk_init().
 *
 * In release mode this does nothing. In debug mode it will initialize
 * memory reports and backtraces, if enabled.
 */
int
mem_init(void);

/*!
 * @brief De-initializes memory tracking. This is called from odbsdk_deinit().
 *
 * In release mode this does nothing. In debug mode this will output the memory
 * report and print backtraces, if enabled.
 * @return Returns the number of memory leaks.
 */
mem_size
mem_deinit(void);

/*!
 * @brief Does the same thing as a normal call to malloc(), but does some
 * additional work to monitor and track down memory leaks.
 */
ODBSDK_PUBLIC_API void*
mem_alloc(mem_size size);

/*!
 * @brief Does the same thing as a normal call to realloc(), but does some
 * additional work to monitor and track down memory leaks.
 */
ODBSDK_PUBLIC_API void*
mem_realloc(void* ptr, mem_size new_size);

/*!
 * @brief Does the same thing as a normal call to fee(), but does some
 * additional work to monitor and track down memory leaks.
 */
ODBSDK_PUBLIC_API void
mem_free(void*);

ODBSDK_PUBLIC_API void
mem_track_allocation(void* p);

ODBSDK_PUBLIC_API void
mem_track_deallocation(void* p);

#endif

