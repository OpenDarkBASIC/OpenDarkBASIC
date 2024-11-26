#pragma once

#include "odb-util/config.h"
#include "odb-util/utf8.h"
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

/* Location logging functions ---------------------------------------------- */
ODBUTIL_PUBLIC_API void
log_flc(const char* filename, const char* source, struct utf8_span location);

enum log_highlight_type
{
    LOG_HIGHLIGHT,
    LOG_INSERT,
    LOG_REMOVE,
};
struct log_highlight
{
    /*! Only used in INSERT mode -- The text to insert at offset loc.off
     * The length of the inserted text must equal loc.len. If unused, set to "".
     * DON'T set to NULL. */
    struct utf8_view        new_text;
    /*! Annotate the highlighted section with additional information.
     * Can be an empty string, but should not be NULL */
    struct utf8_view        annotation;
    /*! If INSERT, then this is the offset in the original text where to insert
     * new_text. Len should be the length of the inserted text.
     * If HIGHLIGHT, then this is the location of the text to highlight. */
    struct utf8_span        loc;
    enum log_highlight_type type;
    /*! The characters to use for underlining the highlighted text. By
     * convention, marker[0]='^', marker[1]='~', marker[2]='<' */
    char                    marker[3];
    /*! Controls the color of the highlighted text. If multiple locations share
     * the same highlight group, they will be colored the same. */
    char                    group;
};
#define LOG_HIGHLIGHT_SENTINAL {{NULL, 0, 0}, {NULL, 0, 0}, {0, 0}, (enum log_highlight_type)0, {'\0', '\0', '\0'}, 0}
#define LOG_IS_SENTINAL(hl) ((hl).marker[0] == '\0')
#define LOG_MARKERS {'^', '~', '<'}

ODBUTIL_PUBLIC_API int
log_excerpt(const char* source, const struct log_highlight* highlights);

static inline int
log_excerpt_1(
    const char* source,
    struct utf8_span location,
    struct utf8_view annotation,
    char group)
{
    struct utf8_view ins = empty_utf8_view();
    struct log_highlight inst[] = {
        {ins, annotation, location, LOG_HIGHLIGHT, LOG_MARKERS, group},
        LOG_HIGHLIGHT_SENTINAL
    };
    return log_excerpt(source, inst);
}

static inline int
log_excerpt_2(
    const char* source,
    struct utf8_span loc1, struct utf8_span loc2,
    struct utf8_view annotation1, struct utf8_view annotation2,
    char group1, char group2)
{
    struct utf8_view ins = empty_utf8_view();
    struct log_highlight hl[] = {
        {ins, annotation1, loc1, LOG_HIGHLIGHT, LOG_MARKERS, group1},
        {ins, annotation2, loc2, LOG_HIGHLIGHT, LOG_MARKERS, group2},
        LOG_HIGHLIGHT_SENTINAL
    };
    return log_excerpt(source, hl);
}

static inline int
log_excerpt_binop(
    const char* source,
    struct utf8_span lhs, struct utf8_span op, struct utf8_span rhs,
    struct utf8_view lhs_text, struct utf8_view rhs_text)
{
    struct utf8_view ins = empty_utf8_view();
    struct log_highlight hl[] = {
        {ins, lhs_text,          lhs, LOG_HIGHLIGHT, {'>', '~', '~'}, 0},
        {ins, empty_utf8_view(), op,  LOG_HIGHLIGHT, {'^', '^', '^'}, 2},
        {ins, rhs_text,          rhs, LOG_HIGHLIGHT, {'~', '~', '<'}, 1},
        LOG_HIGHLIGHT_SENTINAL
    };
    if (lhs.len == 1)
        hl[0].marker[0] = '^';
    if (rhs.len == 1)
        hl[2].marker[0] = '^';
    return log_excerpt(source, hl);
}

/* Memory functions --------------------------------------------------------- */
static inline int
log_oom(size_t bytes, const char* func_name)
{ log_err("Failed to allocate %lu bytes in %s\n", bytes, func_name); return -1; }

/* clang-format on */
