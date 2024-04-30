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
ospath(void)
{
    struct ospath path = { utf8() };
    return path;
}

static inline void
ospath_free(struct ospath* path)
{
    utf8_free(&path->str);
}

ODBSDK_PUBLIC_API int
ospath_set(struct ospath* path, struct utf8_view str);

ODBSDK_PUBLIC_API int
ospath_join(struct ospath* path, struct ospath_view other);

