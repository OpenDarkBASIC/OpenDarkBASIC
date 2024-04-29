#pragma once

#include "odb-sdk/config.h"
#include <stdarg.h>

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(3, 4) void
log_raw(const char* severity, const char* group, const char* fmt, ...);
ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(3, 0) void
log_vraw(const char* severity, const char* group, const char* fmt, va_list ap);

/* General logging functions ----------------------------------------------- */
ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_dbg(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw("[{d:Debug}] ", group, fmt, ap); va_end(ap); }
ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vdbg(const char* group, const char* fmt, va_list ap)
{ log_vraw("[{d:Debug}] ", group, fmt, ap); }

ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_info(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw("[{i:Info }] ", group, fmt, ap); va_end(ap); }
ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vinfo(const char* group, const char* fmt, va_list ap)
{ log_vraw("[{i:Info }] ", group, fmt, ap); }

ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_note(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw("[{n:Note }] ", group, fmt, ap); va_end(ap); }
ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vnote(const char* group, const char* fmt, va_list ap)
{ log_vraw("[{n:Note }] ", group, fmt, ap); }

ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_warn(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw("[{w:Warn }] ", group, fmt, ap); va_end(ap); }
ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vwarn(const char* group, const char* fmt, va_list ap) 
{ log_vraw("[{w:Warn }] ", group, fmt, ap); }

ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_err(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw("[{e:Error}] ", group, fmt, ap); va_end(ap); }
ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_verr(const char* group, const char* fmt, va_list ap)
{ log_vraw("[{e:Error}] ", group, fmt, ap); }

/* SDK logging functions --------------------------------------------------- */
ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_sdk_note(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vnote("[{n:sdk}] ", fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_sdk_warn(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vwarn("[{w:sdk}] ", fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_sdk_err(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_verr("[{e:sdk}] ", fmt, ap); va_end(ap); }

