#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/type.h"

struct ast;
struct cmd_list;

void
ast_swap_node_idxs(struct ast* ast, ast_id n1, ast_id n2);
void
ast_swap_node_values(struct ast* ast, ast_id n1, ast_id n2);

/*!
 * @brief Creates a new node of an identifier.
 */
ast_id
ast_dup_identifier(struct ast** astp, ast_id identifier);
ast_id
ast_dup_lvalue(struct ast** ast, ast_id lvalue);

/*! Perform a deep-copy of a subtree and return the node root node of the new
 * tree, or -1 on failure */
ast_id
ast_dup_subtree(struct ast** ast, ast_id node);
ast_id
ast_dup_subtree_into(
    struct ast**      dst_astp,
    struct utf8*      dst_source,
    const struct ast* src_ast,
    ast_id            node,
    const char*       src_source);

ODBCOMPILER_PUBLIC_API void
ast_delete_node(struct ast* ast, ast_id node);
void
ast_delete_tree(struct ast* ast, ast_id node);

/*!
 * @brief Removes all nodes that have been deleted with @see ast_delete_node()
 *
 * Many semantic checks that modify the tree depend on the various ast_id's not
 * changing during modification. To get around this, when
 * @see ast_delete_node() is called, the node is marked with a special value
 * AST_ID (@see ast_type) and must be cleaned up later with this function.
 */
ODBCOMPILER_PUBLIC_API void
ast_gc(struct ast* ast);

ODBCOMPILER_PUBLIC_API ast_id
ast_find_parent(const struct ast* ast, ast_id node);

/*!
 * Returns true if "node" is found in the subtree starting (and including) node
 * "root"
 */
int
ast_is_in_subtree_of(const struct ast* ast, ast_id node, ast_id root);

int
ast_trees_equal(
    const char* source, const struct ast* ast, ast_id n1, ast_id n2);

void
ast_set_subtree_type(struct ast* ast, ast_id node, union type type);

void
ast_set_subtree_scope(struct ast* ast, ast_id node, enum scope scope);
