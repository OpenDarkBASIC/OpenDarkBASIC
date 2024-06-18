#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/utf8.h"
#include <stdarg.h>

typedef void (*log_write_func)(const char* fmt, va_list ap);

struct log_interface
{
    ODBSDK_PRINTF_FORMAT(1, 0)
    void (*write)(const char* fmt, va_list ap);
    char use_color;
};

void
log_init(void);

ODBSDK_PUBLIC_API struct log_interface
log_configure(struct log_interface iface);

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
ODBSDK_PRINTF_FORMAT(2, 3) static inline int
log_err(const char* group, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_verr(group, fmt, ap); va_end(ap); return -1; }

/* Location logging functions ---------------------------------------------- */
ODBSDK_PRINTF_FORMAT(5, 0) ODBSDK_PUBLIC_API void
log_vflc(const char* severity, const char* filename, const char* source, struct utf8_span location, const char* fmt, va_list ap);
ODBSDK_PRINTF_FORMAT(5, 6) static inline void
log_flc(const char* severity, const char* filename, const char* source, struct utf8_span location, const char* fmt, ...)
{ va_list ap; va_start(ap, fmt); log_vflc(severity, filename, source, location, fmt, ap); va_end(ap); }

ODBSDK_PUBLIC_API int
log_excerpt(const char* filename, const char* source, struct utf8_span location, const char* annotation);

ODBSDK_PUBLIC_API int
log_excerpt2(
    const char* filename, const char* source,
    struct utf8_span loc1, struct utf8_span loc2,
    const char* annotation1, const char* annotation2);

ODBSDK_PUBLIC_API int
log_binop_excerpt(
    const char* filename, const char* source,
    struct utf8_span lhs, struct utf8_span op, struct utf8_span rhs,
    const char* lhs_text, const char* rhs_text);

ODBSDK_PUBLIC_API ODBSDK_PRINTF_FORMAT(2, 3) void
log_excerpt_note(int gutter_indent, const char* fmt, ...);

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

/* Memory functions --------------------------------------------------------- */
static inline int
log_oom(size_t bytes, const char* func_name)
{ log_sdk_err("Failed to allocate %lu bytes in %s\n", bytes, func_name); return -1; }

