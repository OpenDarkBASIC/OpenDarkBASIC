#include "odb-compiler/sdk/type.h"

char
type_to_char(enum type type)
{
    return "\0"
#define X(name, c) #c
        TYPE_LIST
#undef X
            [type];
}

enum type
type_from_char(char c)
{
    switch (c)
    {
#define X(name, c)                                                             \
    case c: return TYPE_##name;
        TYPE_LIST;
#undef X
    }

    return TYPE_INVALID;
}

const char*
type_to_db_name(enum type type)
{
    switch (type)
    {
        case TYPE_INVALID: break;
        case TYPE_VOID: return "VOID";
        case TYPE_I64: return "DOUBLE INTEGER";
        case TYPE_U32: return "DWORD";
        case TYPE_I32: return "INTEGER";
        case TYPE_U16: return "WORD";
        case TYPE_U8: return "BYTE";
        case TYPE_BOOL: return "BOOLEAN";
        case TYPE_F32: return "FLOAT";
        case TYPE_F64: return "DOUBLE";
        case TYPE_STRING: return "STRING";
        case TYPE_ARRAY: return "ARRAY";
        case TYPE_LABEL: break;
        case TYPE_DABEL:
        case TYPE_ANY:
        case TYPE_USER_DEFINED_VAR_PTR: break;
    }

    return "(unknown type)";
}

enum type_conversion_result
type_convert(enum type from, enum type to)
{
    /* clang-format off */
    static enum type_conversion_result rules[16][16] = {
/*       TO */
/*FROM     0 R D L W Y B F O S H P Q X E */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, /* INVALID */
/* 0 */ {0,1,0,0,0,0,0,0,0,0,0,0,0,0,4,0}, /* VOID */
/* R */ {0,0,1,2,2,2,2,4,5,5,0,0,0,0,4,0}, /* LONG */
/* D */ {0,0,1,1,3,2,2,4,5,5,0,0,0,0,4,0}, /* DWORD */
/* L */ {0,0,1,3,1,2,2,4,5,5,0,0,0,0,4,0}, /* INTEGER */
/* W */ {0,0,1,1,1,1,2,4,5,5,0,0,0,0,4,0}, /* WORD */
/* Y */ {0,0,1,1,1,1,1,4,5,5,0,0,0,0,4,0}, /* BYTE */
/* B */ {0,0,6,6,6,6,6,1,6,6,0,0,0,0,4,0}, /* BOOLEAN */
/* F */ {0,0,2,2,2,2,2,4,1,1,0,0,0,0,4,0}, /* FLOAT */
/* O */ {0,0,2,2,2,2,2,4,2,1,0,0,0,0,4,0}, /* DOUBLE */
/* S */ {0,0,0,0,0,0,0,0,0,0,1,0,0,0,4,0}, /* STRING */
/* H */ {0,0,0,0,0,0,0,0,0,0,0,1,0,0,4,0}, /* ARRAY */
/* P */ {0,0,0,0,0,0,0,0,0,0,0,0,1,0,4,0}, /* LABEL */
/* Q */ {0,0,0,0,0,0,0,0,0,0,0,0,0,1,4,0}, /* DLABEL */
/* X */ {0,4,4,4,4,4,4,4,4,4,4,4,4,4,1,0}, /* ANY (reinterpret)*/
/* E */ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, /* USER DEFINED */
    };
    /* clang-format on */
    return rules[from][to];
}
