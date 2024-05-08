#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/ospath.h"

struct mfile
{
    void* address;
    int   size;
};

/*!
 * \brief Memory-maps a file in copy-on-write mode. If the memory is written
 * to, then the data is copied (COW) and no changes are written back to the
 * original file.
 * \param[out] mf Pointer to mfile structure. Struct can be uninitialized.
 * \param[in] file Utf8 encoded file path.
 * \param[in] padding Extra bytes of padding to add to the end of the file
 * mapping. This is required to set up the FLEX buffer, because FLEX expects to
 * find two NULL bytes at the end.
 * \return Returns 0 on success, negative on failure.
 */
ODBSDK_PUBLIC_API int
mfile_map_cow_with_extra_padding(
    struct mfile* mf, struct ospath_view file, int padding);

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
