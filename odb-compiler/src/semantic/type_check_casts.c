#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include <assert.h>

enum type
type_check_cast(
    struct ast* ast, ast_id cast, const char* filename, const char* source)
{
    ODBUTIL_DEBUG_ASSERT(cast > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_semantic_err("type: %d\n", ast_node_type(ast, cast)));

    ast_id expr = ast->nodes[cast].cast.expr;

    enum type source_type = ast_type_info(ast, expr);
    enum type target_type = ast_type_info(ast, cast);

    switch (type_convert(source_type, target_type))
    {
        case TC_ALLOW: return target_type;

        case TC_DISALLOW:
            log_flc_err(
                filename,
                source,
                ast_loc(ast, cast),
                "Cannot cast from {emph1:%s} to {emph2:%s}: Types are "
                "incompatible\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source,
                ast_loc(ast, expr),
                ast_loc(ast, cast),
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;

        case TC_TRUNCATE:
            log_flc_warn(
                filename,
                source,
                ast_loc(ast, cast),
                "Value is truncated when converting from {emph1:%s} to "
                "{emph2:%s} "
                "in expression\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source,
                ast_loc(ast, expr),
                ast_loc(ast, cast),
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;

        case TC_SIGN_CHANGE:
        case TC_TRUENESS:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION:
            log_flc_warn(
                filename,
                source,
                ast_loc(ast, cast),
                "Implicit conversion from {emph1:%s} to {emph2:%s} in "
                "expression\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source,
                ast_loc(ast, expr),
                ast_loc(ast, cast),
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;
    }

    return target_type;
}
