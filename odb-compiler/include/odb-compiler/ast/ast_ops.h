#pragma once

#include "odb-compiler/sdk/type.h"

struct ast;
struct cmd_list;
struct db_source;

void
ast_set_root(struct ast* ast, int node);
void
ast_swap_node_idxs(struct ast* ast, int n1, int n2);
void
ast_swap_node_values(struct ast* ast, int n1, int n2);

/*!
 * @brief Creates a new node of an lvalue, such as an identifier.
 */
int
ast_dup_lvalue(struct ast* ast, int lvalue);

void
ast_delete_node(struct ast* ast, int node);
void
ast_delete_tree(struct ast* ast, int node);
void
ast_gc(struct ast* ast);

int
ast_find_parent(const struct ast* ast, int node);

/*!
 * Returns true if "node" is found in the subtree starting (and including) node
 * "root"
 */
int
ast_is_in_subtree_of(const struct ast* ast, int node, int root);

int
ast_trees_equal(
    const struct db_source* source,
    const struct ast*       a1,
    int                     n1,
    const struct ast*       a2,
    int                     n2);

/*!
 * Finds the most general type following the type promotion rules of a given
 * expression. For example, if we have the expression "3 + 5.6", then this
 * function will end up returning TYPE_DOUBLE, because the result of this
 * operation results in "3" being promoted to a double.
 *
 * Generally, it only makes sense to call this on nodes that return values, i.e.
 * nodes that are expressions. Since commands can appear in expressions, the
 * command list must be passed into find out the return type.
 */
enum type
ast_typeof(const struct ast* ast, int expr, const struct cmd_list* cmds);
