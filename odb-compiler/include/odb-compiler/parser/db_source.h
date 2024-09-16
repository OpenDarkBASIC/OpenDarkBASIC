#pragma once

#include "odb-compiler/config.h"
#include "odb-util/ospath.h"
#include "odb-util/utf8.h"

/*!
 * @brief Represents The contents of a source file.
 *   1) If you want to load a file from the filesystem, use
 *      @see db_source_open_file(). The file must be closed again with
 *      @see db_source_close().
 *   2) If you want to parse a temporary string, use
 *      @see db_source_open_string(). This will copy the string into an internal
 *      memory buffer. The source must be closed again with
 *      @see db_source_close().
 *   3) If you are managing the lifetime of the input string, then use
 *      @see db_source_open_string() to point db_source at it. You do not need
 *      to free anything in this case.
 *
 * This abstraction is required to hide some parser details.
 *   1) FLEX requires an "EOB" marker at the end of its buffers. This is
 *      appended when using the various open() functions below.
 *   2) Throughout the compilation process, any text extracted from the parsing
 *      stage is referenced through @see utf8_span, which is an offset/length
 *      into the source text. Thus, it's necessary to keep the file around for
 *      the duration of the compilation process.
 */
struct db_source
{
    struct utf8 text;
};

/*!
 * @brief Use this if you want to open a file for parsing from the filesystem.
 * @param[out] s Pointer to a db_source structure to fill in. Should be
 * uninitialized, as this will overwrite any previous state.
 * @param[in] filepath Path to a source file to open.
 * @return Returns 0 on success, negative on error.
 */
ODBCOMPILER_PUBLIC_API int
db_source_open_file(struct db_source* s, struct ospathc filepath);

/*!
 * @brief Use this if you need to parse a temporary string. The string is copied
 * into an internal buffer and needs to be freed later using
 * @see db_source_close().
 * @param[out] s Pointer to a db_source structure to fill in. Should be
 * uninitialized, as this will overwrite any previous state.
 * @param[in] str The string to copy for parsing.
 * @return Returns 0 on success, negative on error.
 */
ODBCOMPILER_PUBLIC_API int
db_source_open_string(struct db_source* s, struct utf8_view str);

/*!
 * @brief Use this to parse a stream, such as stdin.
 * @note This function won't return until it encounters an EOF. The entire
 * contents of the input stream are read and copied into a buffer for parsing.
 * @param[out] s Pointer to a db_source structure to fill in. Should be
 * uninitialized, as this will overwrite any previous state.
 * @param[in] fd The stream to read for parsing.
 * @return Returns 0 on success, negative on error.
 */
ODBCOMPILER_PUBLIC_API int
db_source_open_stream(struct db_source* s, FILE* fd);

/*!
 * @brief Use this if you are managing the lifetime of the string to parse. This
 * will simply point the db_source struct to the input string. There is no need
 * to free the db_source structure.
 * @note This will modify the input string by placing an extra NULL byte at the
 * end. This is required for FLEX to function.
 * @param[out] s Pointer to a db_source structure to fill in. Should be
 * uninitialized, as this will overwrite any previous state.
 * @param[in] str The string to be referenced for parsing. Must outlive the
 * db_source structure.
 * @return Returns 0 on success, negative on error.
 */
ODBCOMPILER_PUBLIC_API int
db_source_ref_string(struct db_source* s, struct utf8* str);

/*!
 * @brief Close a source.
 */
ODBCOMPILER_PUBLIC_API void
db_source_close(struct db_source* s);

