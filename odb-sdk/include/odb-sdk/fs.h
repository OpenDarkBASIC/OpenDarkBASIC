#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/ospath.h"

/*!
 * @brief Returns the absolute path to the executable file calling this
 * function, independent of the current working directory.
 * @note The returned path needs to be freed using @see ospath_deinit().
 * @param[out] path Structure receiving the resulting path. Must be initialized.
 * @return Returns 0 on success, negative on error;
 */
ODBSDK_PUBLIC_API int
fs_get_path_to_self(struct ospath* path);

ODBSDK_PUBLIC_API int
fs_list(
    struct ospathc path,
    int                (*on_entry)(const char* name, void* user),
    void*              user);

ODBSDK_PUBLIC_API int
fs_file_exists(struct ospathc path);

ODBSDK_PUBLIC_API int
fs_dir_exists(struct ospathc path);

ODBSDK_PUBLIC_API int
fs_make_dir(const char* path);

ODBSDK_PUBLIC_API int
fs_remove_file(const char* path);

ODBSDK_PUBLIC_API struct empty_ospath
fs_appdata_dir(void);
