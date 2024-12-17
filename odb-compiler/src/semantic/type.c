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
node_to_type(int ast_node)
{
    union type t;
    t.ast_node = ast_node + last_enum_idx() + 1;
    return t;
}

int
type_to_node(union type type)
{
    return type.ast_node - last_enum_idx() - 1;
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

static ast_id
node_to_identifier(const struct ast* ast, ast_id node)
{
    switch (ast_node_type(ast, node))
    {
        case AST_UDT_DECL: return ast->nodes[node].udt_decl.type_identifier;
        case AST_DIM_DECL1:
            node = ast->nodes[node].dim_decl1.dim_decl2;
            return ast->nodes[node].dim_decl2.identifier;
        case AST_FUNC1: return ast->nodes[node].func1.identifier;
        case AST_FUNC_POLY:
            node = ast->nodes[node].func_poly.func;
            return ast->nodes[node].func1.identifier;
        default: break;
    }

    ODBUTIL_DEBUG_ASSERT(0, log_err("type: %d\n", ast_node_type(ast, node)));
    return -1;
}

struct utf8_view
type_name(union type type, const struct ast* ast, const char* source)
{
    ast_id node, ident;

    if (type_is_primitive(type))
        return cstr_utf8_view(primitive_type_name(type.primitive));

    node = type_to_node(type);
    ident = node_to_identifier(ast, node);
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
/* R */ {0,1,2,2,2,2,4,5,5,0}, /* LONG */
/* D */ {0,1,1,3,2,2,4,5,5,0}, /* DWORD */
/* L */ {0,1,3,1,2,2,4,5,5,0}, /* INTEGER */
/* W */ {0,1,1,1,1,2,4,5,5,0}, /* WORD */
/* Y */ {0,1,1,1,1,1,4,5,5,0}, /* BYTE */
/* B */ {0,6,6,6,6,6,1,5,5,0}, /* BOOLEAN */
/* F */ {0,2,2,2,2,2,4,1,1,0}, /* FLOAT */
/* O */ {0,2,2,2,2,2,4,2,1,0}, /* DOUBLE */
/* S */ {0,0,0,0,0,0,0,0,0,1}, /* STRING */
    };
    /* clang-format on */

    if (type_is_primitive(from) && type_is_primitive(to))
        return rules[from.ast_node - 1][to.ast_node - 1];

    if (from.ast_node == to.ast_node)
        return TC_ALLOW;

    return TC_DISALLOW;
}
