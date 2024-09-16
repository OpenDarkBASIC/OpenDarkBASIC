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

#include "odb-util/config.h"
#include "odb-util/utf8.h"

struct ospath
{
    struct utf8 str;
};

struct ospathc
{
    struct utf8c str;
    utf8_idx     len;
};

static inline struct ospath
empty_ospath(void)
{
    struct ospath path = {empty_utf8()};
    return path;
}

static inline struct ospathc
empty_ospathc(void)
{
    struct ospathc path = {empty_utf8c(), 0};
    return path;
}

static inline struct ospathc
ospathc(struct ospath path)
{
    struct ospathc pathc = {utf8_utf8c(path.str), path.str.len};
    return pathc;
}

static inline struct ospathc
utf8_ospathc(struct utf8 str)
{
    struct ospathc pathc = {utf8_utf8c(str), str.len};
    return pathc;
}

static inline struct ospathc
cstr_ospathc(const char* cstr)
{
    struct ospathc pathc = {cstr_utf8c(cstr), (utf8_idx)strlen(cstr)};
    return pathc;
}

static inline const char*
ospathc_cstr(struct ospathc path)
{
    return utf8c_cstr(path.str);
}

static inline struct utf8_view
ospathc_view(struct ospathc path)
{
    struct utf8_view view = {utf8c_cstr(path.str), 0, path.len};
    return view;
}

static inline const char*
ospath_cstr(struct ospath path)
{
    return utf8_cstr(path.str);
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

/* Modifying functions ----------------------------------------------------- */

ODBUTIL_PUBLIC_API int
ospath_set_utf8(struct ospath* path, struct utf8_view str);
static inline int
ospath_set(struct ospath* path, struct ospathc newpath)
{
    struct utf8_view view = {utf8c_cstr(newpath.str), 0, newpath.len};
    return ospath_set_utf8(path, view);
}
static inline int
ospath_set_cstr(struct ospath* path, const char* cstr)
{
    return ospath_set(path, cstr_ospathc(cstr));
}

/*!
 * @brief
 */
ODBUTIL_PUBLIC_API int
ospath_join(struct ospath* path, struct ospathc trailing);
static inline int
ospath_join_cstr(struct ospath* path, const char* cstr)
{
    return ospath_join(path, cstr_ospathc(cstr));
}

ODBUTIL_PUBLIC_API void
ospath_dirname(struct ospath* path);

ODBUTIL_PUBLIC_API void
ospath_filename(struct ospath* path);

ODBUTIL_PUBLIC_API void
ospathc_filename(struct ospathc* path);

static inline void
ospath_remove_ext(struct ospath* path)
{
    utf8_remove_ext(&path->str);
}

/* Query/Test functions ---------------------------------------------------- */

static inline int
ospath_ends_with(struct ospathc path, struct utf8_view str)
{
    return utf8_ends_with(ospathc_view(path), str);
}
static inline int
ospath_ends_with_cstr(struct ospathc path, const char* cstr)
{
    return ospath_ends_with(path, cstr_utf8_view(cstr));
}

static inline int
ospath_ends_with_i(struct ospathc path, struct utf8_view str)
{
    return utf8_ends_with_i(ospathc_view(path), str);
}
static inline int
ospath_ends_with_i_cstr(struct ospathc path, const char* cstr)
{
    return ospath_ends_with_i(path, cstr_utf8_view(cstr));
}

static inline int
ospathc_equal(struct ospathc p1, struct ospathc p2)
{
    return p1.len == p2.len
           && memcmp(p1.str.data, p2.str.data, (size_t)p1.len) == 0;
}
