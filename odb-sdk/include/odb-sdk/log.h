#pragma once

#include "odb-sdk/config.h"

ODBSDK_PUBLIC_API void
log_set_prefix(const char* prefix);
ODBSDK_PUBLIC_API void
log_set_colors(const char* set, const char* clear);

ODBSDK_PUBLIC_API void
log_file_open(const char* log_file);
ODBSDK_PUBLIC_API void
log_file_close(void);

/* General logging functions ----------------------------------------------- */
ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_raw(const char* fmt, ...);

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_dbg(const char* fmt, ...);

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_info(const char* fmt, ...);

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_warn(const char* fmt, ...);

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_err(const char* fmt, ...);

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_note(const char* fmt, ...);

/* Specific logging functions ---------------------------------------------- */
ODBSDK_PRIVATE_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_mem_warn(const char* fmt, ...);
ODBSDK_PRIVATE_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_mem_err(const char* fmt, ...);
ODBSDK_PRIVATE_API ODBSDK_PRINTF_FORMAT(1, 2) void
log_mem_note(const char* fmt, ...);
