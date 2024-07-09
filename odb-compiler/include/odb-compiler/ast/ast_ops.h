#pragma once

#include "odb-compiler/parser/db_source.h"

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
    struct db_source source, const struct ast* ast, int n1, int n2);
