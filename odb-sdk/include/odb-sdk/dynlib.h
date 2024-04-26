#pragma once

#include "odb-sdk/config.h"
#include "odb-sdk/str.h"

/*!
 * \brief Appends a path to search for shared libraries.
 *
 * This is required for plugins that ship with their own shared library
 * dependencies. If the search path is not updated, then loading these plugins
 * will fail because the dynamic linker won't be ablve to find its dependencies.
 *
 * On Windows this calls SetDllDirectory()
 * On Linux this appends a path to LD_LIBRARY_PATH
 * \param[in] path Search path to add.
 * \return Returns 0 on success. Negative on error.
 */
ODBSDK_PUBLIC_API int
dynlib_add_path(const char* path);

/*!
 * \brief Loads a shared library and returns its handle.
 * \param[in] file_name UTF-8 path to file to load.
 * \return Returns a handle on success, NULL on failure.
 */
ODBSDK_PUBLIC_API void*
dynlib_open(const char* file_name);

/*!
 * \brief Unloads a previously loaded shared library.
 * \param[in] handle Handle returned by dynlib_open().
 */
ODBSDK_PUBLIC_API void
dynlib_close(void* handle);

/*!
 * \brief Looks up the address of the specified symbol and returns it.
 * \param[in] handle Handle returned by dynlib_open().
 * \param[in] name Symbol name
 * \return Returns the address to the symbol if successful or NULL on error.
 */
ODBSDK_PUBLIC_API void*
dynlib_symbol_addr(void* handle, const char* name);

ODBSDK_PUBLIC_API int
dynlib_symbol_table(void* handle, int (*on_symbol)(const char* sym, void* user), void* user);

/*!
 * \brief Returns the library's symbol table. These are symbols that have been
 * exported (on Windows) or otherwise made visible, and can be loaded using
 * dynlib_symbol_addr().
 * \note On Linux, shared libraries often have a lot of public symbols. Consider
 * using dynlib_symbol_table_filtered() instead to save on memory/time.
 * \param[in] handle Handle returned by dynlib_open().
 * \param[out] sl String list to receive the list of symbols. Must be initialized.
 * \note This function does not clear the string list. Make sure if you are
 * re-using string lists to first clear them, otherwise old results will
 * persist.
 * \return Returns 0 on success and negative on error.
 */
ODBSDK_PUBLIC_API int
dynlib_symbol_table_strlist(void* handle, struct strlist* sl);

/*!
 * \brief Returns the symbols from the library matching a custom predicate.
 * These are symbols that have been exported (on Windows) or otherwise made
 * visible, and can be loaded using dynlib_symbol_addr().
 * \param[in] handle Handle returned by dynlib_open().
 * \param[out] sl String list to receive the list of symbols. Must be initialized.
 * \note This function does not clear the string list. Make sure if you are
 * re-using string lists to first clear them, otherwise old results will
 * persist.
 * \param[in] match For every symbol found, this callback function will be
 * called with "str" as the name of the symbol. The callback function can
 * return "true" (non-zero) to add the symbol to the output string list, or
 * "false" (zero) to ignore the symbol.
 * \param[in] data A user data pointer. This gets passed to the second argument
 * of the callback function "match". Can be NULL.
 * \return Returns 0 on success and negative on error.
 */
ODBSDK_PUBLIC_API int
dynlib_symbol_table_strlist_filtered(
        void* handle,
        struct strlist* sl,
        int (*match)(struct str_view str, const void* data),
        const void* data);

/*!
 * \brief Returns the number of symbols in the symbol table.
 * \note This is rather slow, and should be cached if used in e.g. a for-loop.
 * \param[in] handle Handle returned by dynlib_open().
 * \return Count, or negative if an error occurred.
 */
ODBSDK_PUBLIC_API int
dynlib_symbol_count(void* handle);

/*!
 * \brief Returns the symbol name at the specified index.
 * \param[in] handle Handle returned by dynlib_open().
 * \param[in] idx 0 to dynlib_symbol_count() - 1
 * \return Null-terminated string of the symbol name.
 */
ODBSDK_PUBLIC_API const char*
dynlib_symbol_at(void* handle, int idx);

#if defined(_WIN32)

ODBSDK_PUBLIC_API int
dynlib_string_count(void* handle);

ODBSDK_PUBLIC_API struct str_view
dynlib_string_at(void* handle, int idx);

#endif
