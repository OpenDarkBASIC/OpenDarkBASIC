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
    X(VOID,    '0')                                                            \
    X(LONG,    'R') /* 8 bytes -- signed int */                                \
    X(DWORD,   'D') /* 4 bytes -- unsigned int */                              \
    X(INTEGER, 'L') /* 4 bytes -- signed int */                                \
    X(WORD,    'W') /* 2 bytes -- unsigned int */                              \
    X(BYTE,    'Y') /* 1 byte  -- unsigned int */                              \
    X(BOOLEAN, 'B') /* 1 byte  -- boolean */                                   \
    X(FLOAT,   'F') /* 4 bytes -- float */                                     \
    X(DOUBLE,  'O') /* 8 bytes -- double */                                    \
    X(STRING,  'S') /* 4/8 bytes -- char* (passed as DWORD on 32-bit) */       \
    X(ARRAY,   'H') /* 4/8 bytes -- Pass array address directly */             \
    X(LABEL,   'P') /* 4 bytes -- ? */                                         \
    X(DABEL,   'Q') /* 4 bytes -- ? */                                         \
    X(ANY,     'X') /* 4 bytes -- (think reinterpret_cast) */                  \
    X(USER_DEFINED_VAR_PTR, 'E') /* 4 bytes */
/* clang-format on */

enum type
{
    TYPE_INVALID,
#define X(name, c) TYPE_##name,
    TYPE_LIST
#undef X
};

enum type_promotion_result
{
    TP_DISALLOW = 0,
    TP_ALLOW = 1,
    TP_NARROWING = 2,
    TP_STRANGE = 3
};

ODBCOMPILER_PUBLIC_API char
type_to_char(enum type type);

ODBCOMPILER_PUBLIC_API enum type
type_from_char(char c);

ODBCOMPILER_PUBLIC_API const char*
type_to_db_name(enum type type);

ODBCOMPILER_PUBLIC_API enum type_promotion_result
type_promote(enum type from, enum type to);

ODBCOMPILER_PUBLIC_API enum type
type_widest(enum type t1, enum type t2);
