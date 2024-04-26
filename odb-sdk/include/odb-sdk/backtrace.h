#pragma once

#include "odb-sdk/config.h"

ODBSDK_PRIVATE_API int
backtrace_init(void);

ODBSDK_PRIVATE_API void
backtrace_deinit(void);

/*!
 * @brief Generates a backtrace.
 * @param[in] size The maximum number of frames to walk.
 * @return Returns an array of char* arrays.
 * @note The returned array must be freed manually with FREE(returned_array).
 */
ODBSDK_PRIVATE_API char**
backtrace_get(int* size);

ODBSDK_PRIVATE_API void
backtrace_free(char** bt);
