#pragma once

#include "odb-compiler/config.h"
#include "odb-util/config.h"
#include <assert.h>

/*!
 * Type information encoding of exported commands from plugins. See
 * https://github.com/TheGameCreators/Dark-Basic-Pro/blob/Initial-Files/Install/Help/documents/1%20Third%20Party%20Commands.htm#L112
 * for a table or command types.
 */
/* clang-format off */
#define TYPE_LIST_DBPRO                                                        \
    /* DBPro types */                                                          \
    X(VOID,         '0')                                                       \
    X(I64,          'R') /* 8 bytes -- signed int */                           \
    X(U32,          'D') /* 4 bytes -- unsigned int */                         \
    X(I32,          'L') /* 4 bytes -- signed int */                           \
    X(U16,          'W') /* 2 bytes -- unsigned int */                         \
    X(U8,           'Y') /* 1 byte  -- unsigned int */                         \
    X(BOOL,         'B') /* 1 byte  -- boolean */                              \
    X(F32,          'F') /* 4 bytes -- float */                                \
    X(F64,          'O') /* 8 bytes -- double */                               \
    X(STRING,       'S') /* 4/8 bytes -- char* (passed as DWORD on 32-bit) */  \
    X(ARRAY,        'H') /* 4/8 bytes -- Pass array address directly */        \
    X(LABEL,        'P') /* 4 bytes -- ? */                                    \
    X(DABEL,        'Q') /* 4 bytes -- ? */                                    \
    X(I64_ARRAY,    'v') /* 4 bytes (DWORD*) */                                \
    X(U32_ARRAY,    'e') /* 4 bytes (DWORD*) */                                \
    X(I32_ARRAY,    'm') /* 4 bytes (DWORD*) */                                \
    X(U16_ARRAY,    'x') /* 4 bytes (DWORD*) */                                \
    X(U8_ARRAY,     'z') /* 4 bytes (DWORD*) */                                \
    X(BOOL_ARRAY,   'c') /* 4 bytes (DWORD*) */                                \
    X(F32_ARRAY,    'g') /* 4 bytes (DWORD*) */                                \
    X(F64_ARRAY,    'u') /* 4 bytes (DWORD*) */                                \
    X(STRING_ARRAY, 't') /* 4 bytes (DWORD*) */                                \
    X(ANY,          'X')    /* 4 bytes -- (think reinterpret_cast) */          \
    X(UDT,          'E')          /* 4 bytes */
/* clang-format on */

#define PRIMITIVE_TYPE_LIST                                                    \
    X(VOID)                                                                    \
    X(I64)                                                                     \
    X(U32)                                                                     \
    X(I32)                                                                     \
    X(U16)                                                                     \
    X(U8)                                                                      \
    X(BOOL)                                                                    \
    X(F32)                                                                     \
    X(F64)                                                                     \
    X(STRING)

struct ast;

enum primitive_type
{
    TYPE_INVALID,
#define X(name) TYPE_##name,
    PRIMITIVE_TYPE_LIST
#undef X
};

/* Either a primitive type, or if the value is greater than the maximum value in
 * the primitive_type enum, is reference into the udt_decl node in the AST. If
 * the original declaration originates from a different AST, it is always copied
 * into the current AST so we don't have to also store the tu_id here. */
union type
{
    enum primitive_type primitive;
    int                 id;
};

enum type_annotation
{
    TA_NONE,
    TA_BOOL = '?',
    TA_U16 = '%',
    TA_I64 = '&',
    TA_F32 = '#',
    TA_F64 = '!',
    TA_STRING = '$'
};

enum type_conversion_result
{
    TC_DISALLOW = 0,
    TC_ALLOW = 1,
    TC_TRUNCATE = 2,
    TC_SIGN_CHANGE = 3,
    TC_TRUENESS = 4,
    TC_INT_TO_FLOAT = 5,
    TC_BOOL_PROMOTION = 6,
};

static inline union type
type_primitive(enum primitive_type primitive)
{
    union type t;
    t.primitive = primitive;
    return t;
}

static inline union type
type_invalid(void)
{
    return type_primitive(TYPE_INVALID);
}

ODBCOMPILER_PUBLIC_API union type
type_udt(int udt_decl);

ODBCOMPILER_PUBLIC_API int
type_udt_decl(union type type);

ODBCOMPILER_PUBLIC_API enum type_annotation
type_to_annotation(union type type);

ODBCOMPILER_PUBLIC_API union type
annotation_to_type(enum type_annotation annotation);

ODBCOMPILER_PUBLIC_API const char*
primitive_type_name(enum primitive_type type);

ODBCOMPILER_PUBLIC_API struct utf8_view
type_name(union type type, const struct ast* ast, const char* source);

ODBCOMPILER_PUBLIC_API enum type_conversion_result
type_convert(union type from, union type to);

ODBCOMPILER_PUBLIC_API enum primitive_type
type_widest(enum primitive_type t1, enum primitive_type t2);

static inline int
type_is_primitive(union type t)
{
    switch (t.primitive)
    {
        case TYPE_INVALID: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case TYPE_VOID:
        case TYPE_I64:
        case TYPE_U32:
        case TYPE_I32:
        case TYPE_U16:
        case TYPE_U8:
        case TYPE_BOOL:
        case TYPE_F32:
        case TYPE_F64:
        case TYPE_STRING: return 1;
    }

    return 0;
}

static inline int
type_is_invalid(union type t)
{
    return t.primitive == TYPE_INVALID;
}

static inline int
type_is_valid(union type t)
{
    return !type_is_invalid(t);
}

static inline int
types_equal(union type t1, union type t2)
{
    return t1.id == t2.id;
}
