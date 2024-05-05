#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/ospath.h"

struct dynlib;

/*!
 * \brief Appends a path to search for shared libraries.
 *
 * This is required for plugins that ship with their own shared library
 * dependencies. If the search path is not updated, then loading these plugins
 * will fail because the dynamic linker won't be able to find its dependencies.
 *
 * On Windows this calls SetDllDirectory()
 * On Linux this appends a path to LD_LIBRARY_PATH
 * \param[in] path Search path to add.
 * \return Returns 0 on success. Negative on error.
 */
ODBSDK_PUBLIC_API int
dynlib_add_path(struct ospath_view path);

/*!
 * \brief Loads a shared library and returns its handle.
 * \param[in] file_name UTF-8 path to file to load.
 * \return Returns a handle on success, NULL on failure.
 */
ODBSDK_PUBLIC_API struct dynlib*
dynlib_open(struct ospath_view filepath);

/*!
 * \brief Unloads a previously loaded shared library.
 * \param[in] handle Handle returned by dynlib_open().
 */
ODBSDK_PUBLIC_API void
dynlib_close(struct dynlib* handle);

/*!
 * \brief Looks up the address of the specified symbol and returns it.
 * \param[in] handle Handle returned by dynlib_open().
 * \param[in] name Symbol name
 * \return Returns the address to the symbol if successful or NULL on error.
 */
ODBSDK_PUBLIC_API void*
dynlib_symbol_addr(struct dynlib* handle, const char* name);

/*!
 * \brief Returns the library's symbol table. These are symbols that have been
 * exported (on Windows) or otherwise made visible, and can be loaded using
 * dynlib_symbol_addr().
 * \note On Linux, shared libraries often have a lot of public symbols. Consider
 * using dynlib_symbol_table_filtered() instead to save on memory/time.
 * \param[in] handle Handle returned by dynlib_open().
 * \param[out] sl String list to receive the list of symbols. Must be
 * initialized. \note This function does not clear the string list. Make sure if
 * you are re-using string lists to first clear them, otherwise old results will
 * persist.
 * \return Returns 0 on success and negative on error.
 */
ODBSDK_PUBLIC_API int
dynlib_symbol_table(
    struct dynlib* handle,
    int            (*on_symbol)(const char* sym, void* user),
    void*          user);

#if defined(_WIN32)

ODBSDK_PUBLIC_API int
dynlib_string_count(struct dynlib* handle);

ODBSDK_PUBLIC_API struct str_view
dynlib_string_at(struct dynlib* handle, int idx);

#endif
