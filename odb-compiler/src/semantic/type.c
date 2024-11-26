#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/log.h"
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>

static int
last_enum_idx(void)
{
    int idx = 0;
#define X(name) idx++;
    PRIMITIVE_TYPE_LIST
#undef X
    return idx;
}

union type
type_udt(int udt_decl)
{
    union type t;
    t.id = udt_decl + last_enum_idx() + 1;
    return t;
}

int
type_udt_decl(union type type)
{
    return type.id - last_enum_idx() - 1;
}

enum type_annotation
type_to_annotation(union type type)
{
    switch (type.primitive)
    {
        case TYPE_INVALID: break;
        case TYPE_VOID: break;

        case TYPE_I64: return TA_I64;
        case TYPE_U16: return TA_U16;
        case TYPE_BOOL: return TA_BOOL;
        case TYPE_F32: return TA_F32;
        case TYPE_F64: return TA_F64;
        case TYPE_STRING: return TA_STRING;

        case TYPE_U32: break;
        case TYPE_I32: break;
        case TYPE_U8: break;
    }

    return TA_NONE;
}

union type
annotation_to_type(enum type_annotation annotation)
{
    switch (annotation)
    {
        case TA_NONE: break;
        case TA_BOOL: return primitive_type(TYPE_BOOL);
        case TA_I64: return primitive_type(TYPE_I64);
        case TA_U16: return primitive_type(TYPE_U16);
        case TA_F64: return primitive_type(TYPE_F64);
        case TA_F32: return primitive_type(TYPE_F32);
        case TA_STRING: return primitive_type(TYPE_STRING);
    }

    return primitive_type(TYPE_I32);
}

const char*
primitive_type_name(enum primitive_type primitive)
{
    switch (primitive)
    {
        case TYPE_INVALID: return "(invalid type)";
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
    }

    return "(unknown type)";
}

struct utf8_view
type_name(union type type, const struct ast* ast, const char* source)
{
    ast_id udt_decl, ident;

    if (type_is_primitive(type))
        return cstr_utf8_view(primitive_type_name(type.primitive));

    udt_decl = type_udt_decl(type);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, udt_decl) == AST_UDT_DECL,
        log_err("type: %d\n", ast_node_type(ast, udt_decl)));
    ident = ast->nodes[udt_decl].udt_decl.type_identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, ident) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, ident)));

    return utf8_span_view(source, ast->nodes[ident].identifier.name);
}

enum type_conversion_result
type_convert(union type from, union type to)
{
    ODBUTIL_DEBUG_ASSERT(from.primitive != TYPE_INVALID, (void)0);
    ODBUTIL_DEBUG_ASSERT(to.primitive != TYPE_INVALID, (void)0);

    /* clang-format off */
    static enum type_conversion_result rules[11][11] = {
/*       TO */
/*FROM   0 R D L W Y B F O S */
/* 0 */ {1,0,0,0,0,0,0,0,0,0}, /* VOID */
/* R */ {1,1,2,2,2,2,4,5,5,0}, /* LONG */
/* D */ {1,1,1,3,2,2,4,5,5,0}, /* DWORD */
/* L */ {1,1,3,1,2,2,4,5,5,0}, /* INTEGER */
/* W */ {1,1,1,1,1,2,4,5,5,0}, /* WORD */
/* Y */ {1,1,1,1,1,1,4,5,5,0}, /* BYTE */
/* B */ {1,6,6,6,6,6,1,5,5,0}, /* BOOLEAN */
/* F */ {1,2,2,2,2,2,4,1,1,0}, /* FLOAT */
/* O */ {1,2,2,2,2,2,4,2,1,0}, /* DOUBLE */
/* S */ {1,0,0,0,0,0,0,0,0,1}, /* STRING */
    };
    /* clang-format on */

    if (type_is_primitive(from) && type_is_primitive(to))
        return rules[from.id - 1][to.id - 1];

    return TC_DISALLOW;
}
