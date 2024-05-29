#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"

struct ast;
struct cmd_list;
struct plugin_list;

typedef int (*semantic_check_func)(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source);

struct semantic_check
{
    const struct semantic_check** depends_on;
    semantic_check_func           execute;
};

ODBCOMPILER_PUBLIC_API int
semantic_checks_run(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source);

/*!
 * The DBPro #constant declaration functions essentially exactly like a C
 * #define macro. The expression within is copied verbatim into every location
 * where the constant is used.
 *
 * This pass expands all references to #constant declarations, and eliminates
 * all AST_CONST_DECL nodes from the AST.
 */
ODBCOMPILER_PUBLIC_API extern const struct semantic_check
    semantic_expand_constant_declarations;

/*!
 * Analyzes all expression trees and ensures that the types of the operands are
 * compatible with each other.
 *
 * NOTE: This pass does not propagate over AST_COMMAND nodes. The parameters of
 * commands are checked by @see semantic_resolve_cmd_overloads. Commands
 * appearing as expressions have their return values checked, though.
 */
ODBCOMPILER_PUBLIC_API extern const struct semantic_check
    semantic_type_check_expressions;

/*!
 * When the parser creates the command node in the AST, it sets the command ID
 * to the first overload, regardless of whether the parameter list makes sense
 * or not.
 *
 * This pass will try to match the types in the parameter list to the argument
 * types expected by the command, and try to find the best overload. If no
 * matching overload is found, then an error is printed.
 *
 * Each AST_COMMAND node is updated with the correct command ID. Codegen can
 * assume thereafter that the parameter list in the AST will match the command's
 * signature.
 */
ODBCOMPILER_PUBLIC_API extern const struct semantic_check
    semantic_resolve_cmd_overloads;

/*!
 * The AST will contain type conversions that are compatible with each other,
 * but do not match exactly. LLVM needs to insert type casts in these locations.
 *
 * This pass will add ast_cast nodes in these locations so that codegen does not
 * have to do any complex type comparisons.
 */
ODBCOMPILER_PUBLIC_API extern const struct semantic_check
    semantic_insert_explicit_type_casts;
