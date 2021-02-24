#pragma once

#include "odb-compiler/config.hpp"

/*!
 * @brief All of the DarkBASIC type annotations that can exist
 *
 * X(enum, chr, str, dbname)
 */
#define ODB_TYPE_ANNOTATION_LIST                     \
    X(DOUBLE_INTEGER, '&', "&", "double integer" )   \
    X(WORD,           '%', "%", "word"           )   \
    X(DOUBLE_FLOAT,   '!', "!", "double float"   )   \
    X(FLOAT,          '#', "#", "float"          )   \
    X(STRING,         '$', "$", "string"         )

namespace odb::ast {

/*!
 * Enum of all type annotations
 */
enum class Annotation : char {
    NONE,
#define X(enum_, chr, str, dbname) enum_,
    ODB_TYPE_ANNOTATION_LIST
#undef X
};

/*!
 * Check if a specific character is a type annotation or not
 */
static inline bool isAnnotation(char c) {
#define X(enum_, chr, str, dbname) if (c == chr) return true;
    ODB_TYPE_ANNOTATION_LIST
#undef X
    return false;
}

ODBCOMPILER_PUBLIC_API const char* typeAnnotationEnumString(Annotation annotation);

}
