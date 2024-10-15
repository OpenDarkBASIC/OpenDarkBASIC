#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include <assert.h>

enum type
type_check_casts(
    struct ast* ast, ast_id cast, const char* filename, const char* source)
{
    ODBUTIL_DEBUG_ASSERT(cast > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[cast].info.node_type == AST_CAST,
        log_semantic_err("type: %d\n", ast->nodes[cast].info.node_type));

    ast_id expr = ast->nodes[cast].cast.expr;

    enum type source_type = ast->nodes[expr].info.type_info;
    enum type target_type = ast->nodes[cast].info.type_info;

    switch (type_convert(source_type, target_type))
    {
        case TC_ALLOW: return target_type;

        case TC_DISALLOW:
            log_flc_err(
                filename,
                source,
                ast->nodes[cast].info.location,
                "Cannot cast from {emph1:%s} to {emph2:%s}: Types are "
                "incompatible\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source,
                ast->nodes[expr].info.location,
                ast->nodes[cast].info.location,
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;

        case TC_TRUNCATE:
            log_flc_warn(
                filename,
                source,
                ast->nodes[cast].info.location,
                "Value is truncated when converting from {emph1:%s} to "
                "{emph2:%s} "
                "in expression\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source,
                ast->nodes[expr].info.location,
                ast->nodes[cast].info.location,
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
                ast->nodes[cast].info.location,
                "Implicit conversion from {emph1:%s} to {emph2:%s} in "
                "expression\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source,
                ast->nodes[expr].info.location,
                ast->nodes[cast].info.location,
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;
    }

    return target_type;
}
