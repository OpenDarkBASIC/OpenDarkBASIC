#pragma once

#include "odb-util/config.h"
#include "odb-util/ospath.h"

/*!
 * @brief Returns the absolute path to the executable file calling this
 * function, independent of the current working directory.
 * @note The returned path needs to be freed using @see ospath_deinit().
 * @param[out] path Structure receiving the resulting path. Must be initialized.
 * @return Returns 0 on success, negative on error;
 */
ODBUTIL_PUBLIC_API int
fs_get_path_to_self(struct ospath* path);

ODBUTIL_PUBLIC_API int
fs_list(
    struct ospathc path,
    int (*on_entry)(const char* name, void* user),
    void* user);

ODBUTIL_PUBLIC_API int
fs_file_exists(struct ospathc path);

ODBUTIL_PUBLIC_API int
fs_dir_exists(struct ospathc path);

/*!
 * @brief Creates a directory on the filesystem. Does not work if the parent
 * directory does not exist.
 * @return Returns -1 if an error occurs, 0 if successful, and 1 if the
 * directory already exists.
 */
ODBUTIL_PUBLIC_API int
fs_make_dir(struct ospathc path);

ODBUTIL_PUBLIC_API int
fs_make_path(struct ospath path);

ODBUTIL_PUBLIC_API int
fs_copy_file(struct ospathc src, struct ospathc dst);

ODBUTIL_PUBLIC_API int
fs_copy_file_if_newer(struct ospathc src, struct ospathc dst);

ODBUTIL_PUBLIC_API int
fs_remove_file(struct ospathc path);

ODBUTIL_PUBLIC_API int
fs_get_appdata_dir(struct ospath* path);

ODBUTIL_PUBLIC_API uint64_t
fs_mtime_ms(struct ospathc path);
