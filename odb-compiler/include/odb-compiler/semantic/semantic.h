#pragma once

#include "odb-compiler/config.h"
#include "odb-util/utf8.h"
#include <stdint.h>

struct ast;
struct cmd_list;
struct db_source;
struct mutex;
struct plugin_list;
struct symbol_table;

typedef int (*semantic_check_func)(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols);

struct semantic_check
{
    semantic_check_func           execute;
    const struct semantic_check** depends_on;
    const char*                   name;
};

ODBCOMPILER_PUBLIC_API int
semantic_check_run(
    const struct semantic_check* check,
    struct ast**                 tus,
    int                          tu_count,
    int                          tu_id,
    struct mutex**               tu_mutexes,
    const struct utf8*           filenames,
    const struct db_source*      sources,
    const struct plugin_list*    plugins,
    const struct cmd_list*       cmds,
    const struct symbol_table*   symbols);

ODBCOMPILER_PUBLIC_API int
semantic_run_essential_checks(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols);

/*!
 * Analyzes all expression trees and ensures that the types of the operands are
 * compatible with each other.
 *
 * NOTE: This pass does not propagate over AST_COMMAND nodes. The parameters of
 * commands are checked by @see semantic_resolve_cmd_overloads. Commands
 * appearing as expressions have their return values checked, though.
 */
ODBCOMPILER_PUBLIC_API extern const struct semantic_check semantic_type_check;

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
 * The parser does not create negative integer/float literals. Instead, it
 * creates a unary negation that preceeds the positive literal.
 *
 * This pass combines these expressions into a single literal.
 */
ODBCOMPILER_PUBLIC_API extern const struct semantic_check
    semantic_unary_literal;

ODBCOMPILER_PUBLIC_API extern const struct semantic_check semantic_loop_for;
ODBCOMPILER_PUBLIC_API extern const struct semantic_check semantic_loop_exit;
ODBCOMPILER_PUBLIC_API extern const struct semantic_check semantic_loop_cont;
ODBCOMPILER_PUBLIC_API extern const struct semantic_check semantic_loop_name;
