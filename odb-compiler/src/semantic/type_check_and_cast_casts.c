#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <assert.h>

enum type
type_check_and_cast_casts(
    struct ast*      ast,
    ast_id           cast,
    const char*      source_filename,
    struct db_source source)
{
    ODBSDK_DEBUG_ASSERT(cast > -1, (void)0);
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[cast].info.node_type == AST_CAST,
        log_semantic_err("type: %d\n", ast->nodes[cast].info.node_type));

    ast_id expr = ast->nodes[cast].cast.expr;

    enum type source_type = ast->nodes[expr].info.type_info;
    enum type target_type = ast->nodes[cast].info.type_info;

    switch (type_promote(source_type, target_type))
    {
        case TP_ALLOW: return target_type;

        case TP_DISALLOW:
            log_flc_err(
                source_filename,
                source.text.data,
                ast->nodes[cast].info.location,
                "Cannot cast from {emph1:%s} to {emph2:%s}: Types are "
                "incompatible\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source.text.data,
                ast->nodes[expr].info.location,
                ast->nodes[cast].info.location,
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;

        case TP_TRUNCATE:
            log_flc_warn(
                source_filename,
                source.text.data,
                ast->nodes[cast].info.location,
                "Value is truncated when converting from {emph1:%s} to {emph2:%s} "
                "in expression\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source.text.data,
                ast->nodes[expr].info.location,
                ast->nodes[cast].info.location,
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;

        case TP_TRUENESS:
        case TP_INT_TO_FLOAT:
        case TP_BOOL_PROMOTION:
            log_flc_warn(
                source_filename,
                source.text.data,
                ast->nodes[cast].info.location,
                "Implicit conversion from {emph1:%s} to {emph2:%s} in expression\n",
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            log_excerpt_2(
                source.text.data,
                ast->nodes[expr].info.location,
                ast->nodes[cast].info.location,
                type_to_db_name(source_type),
                type_to_db_name(target_type));
            break;
    }

    return target_type;
}
