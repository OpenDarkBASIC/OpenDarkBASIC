#pragma once

#include "odb-util/config.h"
#include <stdarg.h>

typedef void (*log_write_func)(const char* fmt, va_list ap);

enum log_level
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_NOTE,
    LOG_WARN,
    LOG_ERR,
};

struct log_interface
{
    ODBUTIL_PRINTF_FORMAT(1, 0)
    void (*write)(const char* fmt, va_list ap);
    char use_color;
};

ODBUTIL_PRIVATE_API int
log_init(void);
ODBUTIL_PRIVATE_API void
log_deinit(void);

ODBUTIL_PUBLIC_API struct log_interface
log_configure(struct log_interface iface);
ODBUTIL_PUBLIC_API int
log_has_color(void);

/* clang-format off */

ODBUTIL_PUBLIC_API ODBUTIL_PRINTF_FORMAT(1, 0) void
log_vraw(const char* fmt, va_list ap);
ODBUTIL_PRINTF_FORMAT(1, 2) static inline void
log_raw(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vraw(fmt, ap); va_end(ap); }

ODBUTIL_PUBLIC_API ODBUTIL_PRINTF_FORMAT(3, 0) void
log_vimpl(
    char is_progress,
    const char* severity,
    const char* fmt,
    va_list ap);

/* General logging functions ----------------------------------------------- */
ODBUTIL_PRINTF_FORMAT(1, 0) static inline void
log_vdbg(const char* fmt, va_list ap)
{ log_vimpl(0, "{d:debug: }", fmt, ap); }
ODBUTIL_PRINTF_FORMAT(1, 2) static inline void
log_dbg(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vdbg(fmt, ap); va_end(ap); }

ODBUTIL_PRINTF_FORMAT(3, 0) ODBUTIL_PUBLIC_API void
log_vprogress(int current, int total, const char* fmt, va_list ap);
ODBUTIL_PRINTF_FORMAT(3, 4) static inline void
log_progress(int current, int total, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vprogress(current, total, fmt, ap); va_end(ap); }

ODBUTIL_PRINTF_FORMAT(1, 0) static inline void
log_vinfo(const char* fmt, va_list ap)
{ log_vimpl(0, "{i:info: }", fmt, ap); }
ODBUTIL_PRINTF_FORMAT(1, 2) static inline void
log_info(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vinfo(fmt, ap); va_end(ap); }

ODBUTIL_PRINTF_FORMAT(1, 0) static inline void
log_vnote(const char* fmt, va_list ap)
{ log_vimpl(0, "{n:note: }", fmt, ap); }
ODBUTIL_PRINTF_FORMAT(1, 2) static inline void
log_note(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vnote(fmt, ap); va_end(ap); }

ODBUTIL_PRINTF_FORMAT(1, 0) static inline void
log_vhelp(const char* fmt, va_list ap)
{ log_vimpl(0, "{h:help: }", fmt, ap); }
ODBUTIL_PRINTF_FORMAT(1, 2) static inline void
log_help(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vhelp(fmt, ap); va_end(ap); }

ODBUTIL_PRINTF_FORMAT(1, 0) static inline void
log_vwarn(const char* fmt, va_list ap) 
{ log_vimpl(0, "{w:warning: }", fmt, ap); }
ODBUTIL_PRINTF_FORMAT(1, 2) static inline void
log_warn(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vwarn(fmt, ap); va_end(ap); }

ODBUTIL_PRINTF_FORMAT(1, 0) static inline int
log_verr(const char* fmt, va_list ap)
{ log_vimpl(0, "{e:error: }", fmt, ap); return -1; }
ODBUTIL_PRINTF_FORMAT(1, 2) static inline int
log_err(const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_verr(fmt, ap); va_end(ap); return -1; }

/* clang-format on */
