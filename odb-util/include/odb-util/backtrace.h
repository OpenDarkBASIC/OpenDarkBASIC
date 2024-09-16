#pragma once

#include "odb-util/config.h"

#if defined(ODBUTIL_MEM_BACKTRACE)
ODBUTIL_PRIVATE_API int
backtrace_init(void);

ODBUTIL_PRIVATE_API void
backtrace_deinit(void);

/*!
 * @brief Generates a backtrace.
 * @param[in] size The maximum number of frames to walk.
 * @return Returns an array of char* arrays.
 * @note The returned array must be freed manually with FREE(returned_array).
 */
ODBUTIL_PRIVATE_API char**
backtrace_get(int* size);

ODBUTIL_PRIVATE_API void
backtrace_free(char** bt);
#else
#define backtrace_init() (0)
#define backtrace_deinit()
#define backtrace_get(x) (NULL)
#define backtrace_free(x)
#endif
