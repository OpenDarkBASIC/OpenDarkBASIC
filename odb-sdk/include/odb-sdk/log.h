#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/utf8.h"
#include <stdarg.h>

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(1, 0) void
log_vraw(const char* fmt, va_list ap);
ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_raw(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw(fmt, ap); va_end(ap); }

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(3, 0) void
log_vimpl(char is_progress, const char* severity, const char* group, const char* fmt, va_list ap);

/* General logging functions ----------------------------------------------- */
ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vdbg(const char* group, const char* fmt, va_list ap)
{ log_vimpl(0, "{d:debug: }", group, fmt, ap); }
ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_dbg(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vdbg(group, fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(4, 0) ODBSDK_PUBLIC_API void
log_vprogress(const char* group, int current, int total, const char* fmt, va_list ap);
ODBSDK_PRINTF_FORMAT(4, 5) static inline void
log_progress(const char* group, int current, int total, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vprogress(group, current, total, fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vinfo(const char* group, const char* fmt, va_list ap)
{ log_vimpl(0, "{i:info:  }", group, fmt, ap); }
ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_info(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vinfo(group, fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vnote(const char* group, const char* fmt, va_list ap)
{ log_vimpl(0, "{n:note:  }", group, fmt, ap); }
ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_note(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vnote(group, fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_vwarn(const char* group, const char* fmt, va_list ap) 
{ log_vimpl(0, "{w:warn:  }", group, fmt, ap); }
ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_warn(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vwarn(group, fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(2, 0) static inline void
log_verr(const char* group, const char* fmt, va_list ap)
{ log_vimpl(0, "{e:error: }", group, fmt, ap); }
ODBSDK_PRINTF_FORMAT(2, 3) static inline void
log_err(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_verr(group, fmt, ap); va_end(ap); }

/* Location logging functions ---------------------------------------------- */
ODBSDK_PRINTF_FORMAT(4, 5) ODBSDK_PUBLIC_API void
log_flc(const char* severity, const char* filename, struct utf8_view location, const char* fmt, ...);

ODBSDK_PUBLIC_API void
log_excerpt(const char* filename, struct utf8_view location);

ODBSDK_PUBLIC_API void
log_binop_excerpt(const char* filename, struct utf8_view lhs, struct utf8_view op, struct utf8_view rhs);

/* SDK logging functions --------------------------------------------------- */
ODBSDK_PRINTF_FORMAT(3, 4) static inline void
log_sdk_progress(int current, int total, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vprogress("[sdk] ", current, total, fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_sdk_info(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vinfo("[sdk] ", fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_sdk_note(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vnote("[sdk] ", fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(1, 2) static inline void
log_sdk_warn(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vwarn("[sdk] ", fmt, ap); va_end(ap); }

ODBSDK_PRINTF_FORMAT(1, 2) static inline int
log_sdk_err(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_verr("[sdk] ", fmt, ap); va_end(ap); return -1; }

/* Parser logging functions ------------------------------------------------- */
ODBSDK_PRINTF_FORMAT(1, 2) static inline int
log_parser_err(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_verr("[parser] ", fmt, ap); va_end(ap); return -1; }

