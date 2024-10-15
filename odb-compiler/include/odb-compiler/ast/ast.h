#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-util/utf8.h"

/*!
 * @brief All of the DarkBASIC operators that can exist
 */
/* clang-format off */
#define BINOP_LIST              \
    X(ADD,           "+")       \
    X(SUB,           "-")       \
    X(MUL,           "*")       \
    X(DIV,           "/")       \
    X(MOD,           "mod")     \
    X(POW,           "^")       \
                                \
    X(LESS_THAN,     "<")       \
    X(LESS_EQUAL,    "<=")      \
    X(GREATER_THAN,  ">")       \
    X(GREATER_EQUAL, ">=")      \
    X(EQUAL,         "=")       \
    X(NOT_EQUAL,     "<>")      \
    X(LOGICAL_OR,    "or")      \
    X(LOGICAL_AND,   "and")     \
    X(LOGICAL_XOR,   "xor")     \
                                \
    X(SHIFT_LEFT,    "<<")      \
    X(SHIFT_RIGHT,   ">>")      \
    X(BITWISE_OR,    "||")      \
    X(BITWISE_AND,   "&&")      \
    X(BITWISE_XOR,   "~~")      \
    X(BITWISE_NOT,   "..")      

#define UNOP_LIST               \
    X(LOGICAL_NOT,  "not")      \
    X(NEGATE,       "-")        \
    X(BITWISE_NOT,  "..")
/* clang-format on */

typedef int32_t ast_id;

enum binop_type
{
#define X(op, tok) BINOP_##op,
    BINOP_LIST
#undef X
};

enum unop_type
{
#define X(op, tok) UNOP_##op,
    UNOP_LIST
#undef X
};

enum scope
{
    SCOPE_GLOBAL,
    SCOPE_LOCAL,
};

enum ast_type
{
    /*! Marks a node as being deleted. All unused nodes are removed using
       ast_gc() */
    AST_GC,
    /*! Linked list of sequential statements */
    AST_BLOCK,
    /*! Linked list of expressions, usually passed as arguments to a function or
       command */
    AST_END,
    AST_ARGLIST,
    AST_PARAMLIST,
    /*! Function call to a plugin. Known in DBPro as a "command" */
    AST_COMMAND,
    /*! Assignment statement, e.g. x=5 */
    AST_ASSIGNMENT,
    /*! Any referrable entity, such as a function name, variable, UDT field,
       etc. */
    AST_IDENTIFIER,
    /*! Binarop operator such as a+b, a-b, a*b, etc. */
    AST_BINOP,
    /*! Unary operator such as -a, !a, etc. */
    AST_UNOP,
    AST_COND,
    AST_COND_BRANCHES,
    AST_LOOP,
    AST_LOOP_BODY,
    AST_LOOP_FOR1,
    AST_LOOP_FOR2,
    AST_LOOP_FOR3,
    AST_LOOP_CONT,
    AST_LOOP_EXIT,
    AST_FUNC_TEMPLATE,
    AST_FUNC,
    AST_FUNC_DECL,
    AST_FUNC_DEF,
    AST_FUNC_EXIT,
    AST_FUNC_OR_CONTAINER_REF,
    AST_FUNC_CALL,
    /*! Boolean literal, either "true" or "false" */
    AST_BOOLEAN_LITERAL,
    /*! A literal between 0 and 255. Maps to uint8_t. */
    AST_BYTE_LITERAL,
    /*! A literal between 0 and 65535. Maps to uint16_t */
    AST_WORD_LITERAL,
    /*! A literal between 0 and 2^32-1. Maps to uint32_t */
    AST_DWORD_LITERAL,
    /*! A literal between -2^31 and 2^31-1. Maps to int32_t */
    AST_INTEGER_LITERAL,
    /*! A literal between -2^63 and 2^63-1. Maps to int64_t */
    AST_DOUBLE_INTEGER_LITERAL,
    AST_FLOAT_LITERAL,
    AST_DOUBLE_LITERAL,
    /*! String. Should be UTF-8 encoded. */
    AST_STRING_LITERAL,
    AST_CAST,
    AST_SCOPE,
};

/* clang-format off */

union ast_node
{
    struct info
    {
        struct utf8_span location;
        int32_t scope_id;
        enum ast_type node_type : 6;
        enum type type_info : 4;
    } info;

    struct base
    {
        struct info info;
        ast_id left;
        ast_id right;
    } base;

    struct {
        struct info info;
        ast_id stmt;
        ast_id next;
    } block;

    struct {
        struct info info;
        ast_id _pad1, _pad2;
    } end;

    struct 
    {
        struct info info;
        ast_id expr;
        ast_id next;
        struct utf8_span combined_location;
    } arglist;

    struct 
    {
        struct info info;
        ast_id identifier;
        ast_id next;
        struct utf8_span combined_location;
    } paramlist;

    struct 
    {
        struct info info;
        ast_id arglist;
        ast_id _pad;
        cmd_id id;
    } cmd;

    struct 
    {
        struct info info;
        ast_id lvalue;
        ast_id expr;
        struct utf8_span op_location;
    } assignment;

    struct 
    {
        struct info info;
        ast_id _pad1, _pad2;
        struct utf8_span name;
        enum type_annotation annotation : 7;
        enum scope scope : 1;
        enum type explicit_type : 4;
    } identifier;

    struct 
    {
        struct info info;
        ast_id left;
        ast_id right;
        struct utf8_span op_location;
        enum binop_type op : 5;
    } binop;

    struct 
    {
        struct info info;
        ast_id expr;
        ast_id _pad;
        enum unop_type op : 3;
    } unop;

    struct {
        struct info info;
        ast_id expr;
        ast_id cond_branches;
    } cond;
    struct {
        struct info info;
        ast_id yes;
        ast_id no;
    } cond_branches;

    struct {
        struct info info;
        ast_id loop_body;
        /* Before semantic, this points to a list of nodes containing details of
         * the for-loop. After semantic, these nodes are removed and the property
         * is set to -1 */
        ast_id loop_for1;
        struct utf8_span name;
        struct utf8_span implicit_name;
    } loop;
    /* The body is split up into the "body" (block of statements) and a piece
     * of code that is executed at the very end of the loop. The info needs to
     * be kept separate because the "continue" keyword is able to override this
     * piece of code in the case of for-loops. */
    struct {
        struct info info;
        ast_id body;
        ast_id post_body;
    } loop_body;
    /* Holds info necessary for semantic to generate error messages.
     * Instances of these nodes are removed from the tree during
     * semantic analysis. It's necessary to split them up like this because all
     * nodes must only contain a max of 2 children. */
    struct {
        struct info info;
        ast_id loop_for2;
        ast_id init;
    } loop_for1;
    struct {
        struct info info;
        ast_id loop_for3;
        ast_id end;
    } loop_for2;
    struct {
        struct info info;
        ast_id step;
        ast_id next;
    } loop_for3;

    struct {
        struct info info;
        ast_id step;
        ast_id _pad;
        struct utf8_span name;
    } cont;

    struct {
        struct info info;
        ast_id _pad1, _pad2;
        struct utf8_span name;
    } loop_exit;

    struct {
        struct info info;
        ast_id decl;
        ast_id def;
    } func_template;

    struct {
        struct info info;
        ast_id decl;
        ast_id def;
    } func;

    struct {
        struct info info;
        ast_id identifier;
        ast_id paramlist;
    } func_decl;

    struct {
        struct info info;
        ast_id body;
        ast_id retval;
    } func_def;

    struct {
        struct info info;
        ast_id retval;
        ast_id _pad;
    } func_exit;

    struct {
        struct info info;
        ast_id identifier;
        ast_id arglist;
    } func_or_container_ref;

    struct {
        struct info info;
        ast_id identifier;
        ast_id arglist;
    } func_call;

    struct {
        struct info info;
        ast_id _pad1, _pad2;
        char is_true;
    } boolean_literal;

    struct {
        struct info info;
        ast_id _pad1, _pad2;
        uint8_t value;
    } byte_literal;
    struct {
        struct info info;
        ast_id _pad1, _pad2;
        uint16_t value;
    } word_literal;
    struct {
        struct info info;
        ast_id _pad1, _pad2;
        int32_t value;
    } integer_literal;
    struct {
        struct info info;
        ast_id _pad1, _pad2;
        uint32_t value;
    } dword_literal;
    struct {
        struct info info;
        ast_id _pad1, _pad2;
        int64_t value;
    } double_integer_literal;

    struct {
        struct info info;
        ast_id _pad1, _pad2;
        float value;
    } float_literal;
    struct {
        struct info info;
        ast_id _pad1, _pad2;
        double value;
    } double_literal;

    struct {
        struct info info;
        ast_id _pad1, _pad2;
        struct utf8_span str;
    } string_literal;

    struct {
        struct info info;
        ast_id expr;
        ast_id _pad;
    } cast;

    struct {
        struct info info;
        ast_id child;
        ast_id _pad;
    } scope;
};

struct ast
{
    ast_id count, capacity;
    ast_id root;
    union ast_node nodes[1];
};

static inline void
ast_init(struct ast** astp)
{
    *astp = NULL;
}

ODBCOMPILER_PUBLIC_API void 
ast_deinit(struct ast* ast);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_ast(struct ast* ast);
ODBCOMPILER_PUBLIC_API void
mem_release_ast(struct ast* ast);
#else
#define mem_acquire_ast(ast)
#define mem_release_ast(ast)
#endif

static inline void
ast_set_root(struct ast* ast, ast_id n)
{
    if (ast)
        ast->root = n;
}

static inline ast_id
ast_count(const struct ast* ast)
    { return ast ? ast->count : 0; }
static inline ast_id
ast_count_unsafe(const struct ast* ast)
    { return ast->count; }
static inline enum ast_type
ast_node_type(const struct ast* ast, ast_id n)
    { return ast->nodes[n].info.node_type; }
static inline enum type
ast_type_info(const struct ast* ast, ast_id n)
    { return ast->nodes[n].info.type_info; }
static inline struct utf8_span
ast_loc(const struct ast* ast, ast_id n)
    { return ast->nodes[n].info.location; }

ast_id ast_dup_node(struct ast** astp, ast_id n);

ast_id ast_block(struct ast** astp, ast_id stmt, struct utf8_span location);
void ast_block_append(struct ast* ast, ast_id block, ast_id append_block);
ast_id ast_block_append_stmt(struct ast** astp, ast_id block, ast_id stmt, struct utf8_span location);
ast_id ast_end(struct ast** astp, struct utf8_span location);
ast_id ast_arglist(struct ast** astp, ast_id expr, struct utf8_span location);
ast_id ast_arglist_append(struct ast** astp, ast_id arglist, ast_id expr, struct utf8_span location);
ast_id ast_paramlist(struct ast** astp, ast_id expr, struct utf8_span location);
ast_id ast_paramlist_append(struct ast** astp, ast_id paramlist, ast_id expr, struct utf8_span location);
ast_id ast_command(struct ast** astp, cmd_id cmd_id, ast_id arglist, struct utf8_span location);
ast_id ast_assign(struct ast** astp, ast_id identifier, ast_id expr, struct utf8_span op_location, struct utf8_span location);
ast_id ast_inc_step(struct ast** astp, ast_id var, ast_id expr, struct utf8_span location);
ast_id ast_inc(struct ast** astp, ast_id var, struct utf8_span location);
ast_id ast_dec_step(struct ast** astp, ast_id var, ast_id expr, struct utf8_span location);
ast_id ast_dec(struct ast** astp, ast_id var, struct utf8_span location);
ast_id ast_identifier(struct ast** astp, struct utf8_span name, enum type_annotation annotation, struct utf8_span location);
ast_id ast_binop(struct ast** astp, enum binop_type op, ast_id left, ast_id right, struct utf8_span op_location, struct utf8_span location);
ast_id ast_unop(struct ast** astp, enum unop_type op, ast_id expr, struct utf8_span location);
ast_id ast_cond(struct ast** astp, ast_id expr, ast_id cond_branches, struct utf8_span location);
ast_id ast_cond_branches(struct ast** astp, ast_id yes, ast_id no, struct utf8_span location);
ast_id ast_loop(struct ast** astp, ast_id body, struct utf8_span name, struct utf8_span implicit_name, struct utf8_span location);
ast_id ast_loop_while(struct ast** astp, ast_id body, ast_id expr, struct utf8_span name, struct utf8_span location);
ast_id ast_loop_until(struct ast** astp, ast_id body, ast_id expr, struct utf8_span name, struct utf8_span location);
ast_id ast_loop_for(struct ast** astp, ast_id body, ast_id init, ast_id end, ast_id step, ast_id next, struct utf8_span name, struct utf8_span location);
ast_id ast_loop_cont(struct ast** astp, struct utf8_span name, ast_id step, struct utf8_span location);
ast_id ast_loop_exit(struct ast** astp, struct utf8_span name, struct utf8_span location);
ast_id ast_func(struct ast** astp, ast_id identifier, ast_id paramlist, ast_id body, ast_id retval, struct utf8_span location);
ast_id ast_func_exit(struct ast** astp, ast_id retval, struct utf8_span location);
ast_id ast_func_or_container_ref(struct ast** astp, ast_id identifier, ast_id arglist, struct utf8_span location);
ast_id ast_boolean_literal(struct ast** astp, char is_true, struct utf8_span location);
ast_id ast_byte_literal(struct ast** astp, uint8_t value, struct utf8_span location);
ast_id ast_word_literal(struct ast** astp, uint16_t value, struct utf8_span location);
ast_id ast_integer_literal(struct ast** astp, int32_t value, struct utf8_span location);
ast_id ast_dword_literal(struct ast** astp, uint32_t value, struct utf8_span location);
ast_id ast_double_integer_literal(struct ast** astp, int64_t value, struct utf8_span location);
ast_id ast_integer_like_literal(struct ast** astp, int64_t value, struct utf8_span location);
ast_id ast_float_literal(struct ast** astp, float value, struct utf8_span location);
ast_id ast_double_literal(struct ast** astp, double value, struct utf8_span location);
ast_id ast_string_literal(struct ast** astp, struct utf8_span str, struct utf8_span location);
ast_id ast_cast(struct ast** astp, ast_id expr, enum type target_type, struct utf8_span location);
ast_id ast_scope(struct ast** astp, ast_id child, struct utf8_span location);
/* clang-format on */
