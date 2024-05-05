/*
 * @brief Provides functions for manipulating and normalizing path strings such
 * that they may be understood by the operating system.
 *
 * Because these strings are typically passed on to operating system functions,
 * they will be formatted differently depending on the platform you're compiling
 * for. For instance, on Windows, creating a path from a string will cause all
 * forward slashes to be replaced by backslashes, but on linux, all backslashes
 * will be replaced by forward slashes.
 *
 * All strings are utf8 encoded. When using OS functions on Windows there is an
 * additional layer that will transparently convert these utf8 paths into utf16
 * paths before passing them on to the Windows API.
 */

#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/utf8.h"

struct ospath
{
    struct utf8       str;
    struct utf8_range range;
};

struct ospath_view
{
    struct utf8_view  str;
    struct utf8_range range;
};

static inline struct ospath
ospath(void)
{
    struct ospath path = {utf8(), utf8_range()};
    return path;
}

static inline struct ospath_view
ospath_view(struct ospath path)
{
    struct ospath_view view = {utf8_view(path.str, path.range), path.range};
    return view;
}

static inline struct ospath_view
cstr_ospath_view(const char* cstr)
{
    struct ospath_view view = {cstr_utf8_view(cstr), cstr_utf8_range(cstr)};
    return view;
}

static inline const char*
ospath_view_cstr(struct ospath_view path)
{
    return utf8_view_cstr(path.str);
}

static inline const char*
ospath_cstr(struct ospath path)
{
    return ospath_view_cstr(ospath_view(path));
}

static inline int
ospath_len(struct ospath path)
{
    return path.range.len;
}

static inline void
ospath_deinit(struct ospath path)
{
    utf8_deinit(path.str);
}

static inline int
ospath_set(struct ospath* path, const struct ospath_view other)
{
    return utf8_set(&path->str, &path->range, other.str, other.range);
}
ODBSDK_PUBLIC_API int
ospath_set_utf8(
    struct ospath* path, struct utf8_view str, struct utf8_range range);
static inline int
ospath_set_cstr(struct ospath* path, const char* cstr)
{
    return ospath_set_utf8(path, cstr_utf8_view(cstr), cstr_utf8_range(cstr));
}

/*!
 * @brief
 */
ODBSDK_PUBLIC_API int
ospath_join(struct ospath* path, struct ospath_view trailing);
static inline int
ospath_join_cstr(struct ospath* path, const char* cstr)
{
    return ospath_join(path, cstr_ospath_view(cstr));
}

static inline int
ospath_ends_with(struct ospath_view path, struct utf8_view str, struct utf8_range range)
{
    return utf8_ends_with(path.str, path.range, str, range);
}
static inline int
ospath_ends_with_cstr(struct ospath_view path, const char* cstr)
{
    return ospath_ends_with(path, cstr_utf8_view(cstr), cstr_utf8_range(cstr));
}

static inline int
ospath_ends_with_i(struct ospath_view path, struct utf8_view str, struct utf8_range range)
{
    return utf8_ends_with_i(path.str, path.range, str, range);
}
static inline int
ospath_ends_with_i_cstr(struct ospath_view path, const char* cstr)
{
    return ospath_ends_with_i(path, cstr_utf8_view(cstr), cstr_utf8_range(cstr));
}
