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
    struct utf8 str;
};

struct ospath_view
{
    struct utf8_view str;
};

static inline struct ospath
empty_ospath(void)
{
    struct ospath path = {empty_utf8()};
    return path;
}

static inline struct ospath_view
empty_ospath_view(void)
{
    struct ospath_view path = {empty_utf8_view()};
    return path;
}

static inline struct ospath_view
ospath_view(struct ospath path)
{
    struct ospath_view view = {utf8_view(path.str)};
    return view;
}

static inline struct ospath_view
cstr_ospath_view(const char* cstr)
{
    struct ospath_view view = {cstr_utf8_view(cstr)};
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
    return path.str.len;
}

static inline void
ospath_deinit(struct ospath path)
{
    utf8_deinit(path.str);
}

static inline int
ospath_set(struct ospath* path, const struct ospath_view other)
{
    return utf8_set(&path->str, other.str);
}
ODBSDK_PUBLIC_API int
ospath_set_utf8(struct ospath* path, struct utf8_view str);
static inline int
ospath_set_cstr(struct ospath* path, const char* cstr)
{
    return ospath_set_utf8(path, cstr_utf8_view(cstr));
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
ospath_ends_with(struct ospath_view path, struct utf8_view str)
{
    return utf8_ends_with(path.str, str);
}
static inline int
ospath_ends_with_cstr(struct ospath_view path, const char* cstr)
{
    return ospath_ends_with(path, cstr_utf8_view(cstr));
}

static inline int
ospath_ends_with_i(struct ospath_view path, struct utf8_view str)
{
    return utf8_ends_with_i(path.str, str);
}
static inline int
ospath_ends_with_i_cstr(struct ospath_view path, const char* cstr)
{
    return ospath_ends_with_i(path, cstr_utf8_view(cstr));
}
