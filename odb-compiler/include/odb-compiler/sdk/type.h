#pragma once

#include "odb-compiler/config.h"

/* clang-format off */
/*!
 * Type information encoding of exported commands from plugins. See
 * https://github.com/TheGameCreators/Dark-Basic-Pro/blob/Initial-Files/Install/Help/documents/1%20Third%20Party%20Commands.htm#L112
 * for a table or command types.
 *
 * ODB shares most types with DBPro.
 */
#define TYPE_LIST                                                              \
    /* DBPro types */                                                          \
    X(VOID,           '0')                                                     \
    X(I64,            'R') /* 8 bytes -- signed int */                         \
    X(U32,            'D') /* 4 bytes -- unsigned int */                       \
    X(I32,            'L') /* 4 bytes -- signed int */                         \
    X(U16,            'W') /* 2 bytes -- unsigned int */                       \
    X(U8,             'Y') /* 1 byte  -- unsigned int */                       \
    X(BOOL,           'B') /* 1 byte  -- boolean */                            \
    X(F32,            'F') /* 4 bytes -- float */                              \
    X(F64,            'O') /* 8 bytes -- double */                             \
    X(STRING,         'S') /* 4/8 bytes -- char* (passed as DWORD on 32-bit) */\
    X(ARRAY,          'H') /* 4/8 bytes -- Pass array address directly */      \
    X(LABEL,          'P') /* 4 bytes -- ? */                                  \
    X(DABEL,          'Q') /* 4 bytes -- ? */                                  \
    X(ANY,            'X') /* 4 bytes -- (think reinterpret_cast) */           \
    X(USER_DEFINED_VAR_PTR, 'E') /* 4 bytes */
/* clang-format on */

enum type
{
    TYPE_INVALID,
#define X(name, c) TYPE_##name,
    TYPE_LIST
#undef X
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

ODBCOMPILER_PUBLIC_API char
type_to_char(enum type type);

ODBCOMPILER_PUBLIC_API enum type
type_from_char(char c);

ODBCOMPILER_PUBLIC_API const char*
type_to_db_name(enum type type);

ODBCOMPILER_PUBLIC_API enum type_conversion_result
type_convert(enum type from, enum type to);

ODBCOMPILER_PUBLIC_API enum type
type_widest(enum type t1, enum type t2);
