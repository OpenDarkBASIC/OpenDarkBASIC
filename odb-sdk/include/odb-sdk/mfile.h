#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/ospath.h"

struct mfile
{
    void* address;
    int   size;
};

/*!
 * \brief Memory-maps a file in read-only mode.
 * \param[out] mf Pointer to mfile structure. Struct can be uninitialized.
 * \param[in] file Utf8 encoded file path.
 * \return Returns 0 on success, negative on failure.
 */
ODBSDK_PUBLIC_API int
mfile_map_read(struct mfile* mf, struct ospathc filepath);

/*!
 * \brief Memory-maps a file for writing. The existing file is overwritten.
 * \param[out] mf Pointer to mfile structure. Struct can be uninitialized.
 * \param[in] file Utf8 encoded file path.
 * \return Returns 0 on success, negative on failure.
 */
ODBSDK_PUBLIC_API int
mfile_map_overwrite(struct mfile* mf, struct ospathc filepath);

/*!
 * @brief Allocates memory using mmap. The memory must be freed again using
 * @see mfile_unmap() if this function succeeds.
 * @param[out] mf Pointer to mfile structure. Struct can be uninitialized.
 * @param[in] size Size, in bytes, of memory allocation.
 * \return Returns 0 on success, negative on failure.
 */
ODBSDK_PUBLIC_API int
mfile_map_mem(struct mfile* mf, int size);

/*! \brief Unmap a previously mapped file. */
ODBSDK_PUBLIC_API void
mfile_unmap(struct mfile* mf);
