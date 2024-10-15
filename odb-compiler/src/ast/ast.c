#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/utf8.h"
#include <assert.h>

static ast_id
ast_grow(struct ast** astp)
{
    struct ast* ast = *astp;
    if (ast == NULL || ast->count == ast->capacity)
    {
        ast_id   new_capacity = ast ? ast->capacity * 2 : 128;
        mem_size header_size = offsetof(struct ast, nodes);
        mem_size nodes_size = sizeof(union ast_node) * new_capacity;

        struct ast* new_ast = mem_realloc(ast, header_size + nodes_size);
        if (new_ast == NULL)
            return log_oom(header_size + nodes_size, "new_node()");

        if (ast == NULL)
            new_ast->count = 0;
        new_ast->capacity = new_capacity;
        ast = new_ast;
        *astp = new_ast;
    }

    return ast->count++;
}

static ast_id
new_node(struct ast** astp, enum ast_type type, struct utf8_span location)
{
    struct ast* ast;
    ast_id      n = ast_grow(astp);
    if (n < 0)
        return -1;
    ast = *astp;

    ast->nodes[n].base.info.location = location;
    ast->nodes[n].base.info.node_type = type;
    ast->nodes[n].base.info.type_info = TYPE_INVALID;
    ast->nodes[n].base.left = -1;
    ast->nodes[n].base.right = -1;

    return n;
}

void
ast_deinit(struct ast* ast)
{
    if (ast)
        mem_free(ast);
}
#if defined(ODBUTIL_MEM_DEBUGGING)
void
mem_acquire_ast(struct ast* ast)
{
    mem_size header, nodes;
    if (ast == NULL)
        return;

    header = offsetof(struct ast, nodes);
    nodes = sizeof(union ast_node) * ast->capacity;
    mem_acquire(ast, header + nodes);
}
void
mem_release_ast(struct ast* ast)
{
    mem_release(ast);
}
#endif

ast_id
ast_dup_node(struct ast** astp, ast_id n)
{
    ast_id dup = ast_grow(astp);
    if (dup < 0)
        return -1;

    memcpy(&(*astp)->nodes[dup], &(*astp)->nodes[n], sizeof(union ast_node));
    return dup;
}

ast_id
ast_block(struct ast** astp, ast_id stmt, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_BLOCK, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(stmt > -1, log_parser_err("stmt: %d\n", stmt));
    ast->nodes[n].block.stmt = stmt;

    return n;
}

void
ast_block_append(struct ast* ast, ast_id block, ast_id append_block)
{
    ODBUTIL_DEBUG_ASSERT(block > -1, log_parser_err("block: %d\n", block));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[block].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[block].info.node_type));
    ODBUTIL_DEBUG_ASSERT(
        append_block > -1, log_parser_err("block: %d\n", block));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[append_block].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[append_block].info.node_type));

    while (ast->nodes[block].block.next != -1)
        block = ast->nodes[block].block.next;

    ast->nodes[block].block.next = append_block;
}
ast_id
ast_block_append_stmt(
    struct ast** astp, ast_id block, ast_id stmt, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_BLOCK, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast_block_append(ast, block, n);
    ast->nodes[n].block.stmt = stmt;

    return n;
}

ast_id
ast_end(struct ast** astp, struct utf8_span location)
{
    return new_node(astp, AST_END, location);
}

ast_id
ast_arglist(struct ast** astp, ast_id expr, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_ARGLIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ast->nodes[n].arglist.expr = expr;

    return n;
}

ast_id
ast_arglist_append(
    struct ast** astp, ast_id arglist, ast_id expr, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_ARGLIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ODBUTIL_DEBUG_ASSERT(
        arglist > -1, log_parser_err("arglist: %d\n", arglist));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[arglist].info.node_type == AST_ARGLIST,
        log_parser_err("type: %d\n", ast->nodes[arglist].info.node_type));

    while (ast->nodes[arglist].arglist.next != -1)
        arglist = ast->nodes[arglist].arglist.next;

    ast->nodes[arglist].arglist.next = n;
    ast->nodes[n].arglist.expr = expr;

    return n;
}

ast_id
ast_paramlist(struct ast** astp, ast_id identifier, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_PARAMLIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        identifier > -1, log_parser_err("expr: %d\n", identifier));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[identifier].info.node_type));

    ast->nodes[n].paramlist.identifier = identifier;
    return n;
}

ast_id
ast_paramlist_append(
    struct ast**     astp,
    ast_id           paramlist,
    ast_id           identifier,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_PARAMLIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        paramlist > -1, log_parser_err("paramlist: %d\n", paramlist));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[paramlist].info.node_type == AST_PARAMLIST,
        log_parser_err("type: %d\n", ast->nodes[paramlist].info.node_type));

    ODBUTIL_DEBUG_ASSERT(
        identifier > -1, log_parser_err("expr: %d\n", identifier));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[identifier].info.node_type));

    while (ast->nodes[paramlist].paramlist.next != -1)
        paramlist = ast->nodes[paramlist].paramlist.next;

    ast->nodes[paramlist].paramlist.next = n;
    ast->nodes[n].paramlist.identifier = identifier;

    return n;
}

ast_id
ast_command(
    struct ast** astp, cmd_id cmd_id, ast_id arglist, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_COMMAND, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(cmd_id > -1, log_parser_err("cmd_id: %d\n", cmd_id));
    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast->nodes[arglist].info.node_type == AST_ARGLIST,
        log_parser_err("type: %d\n", ast->nodes[arglist].info.node_type));

    ast->nodes[n].cmd.id = cmd_id;
    ast->nodes[n].cmd.arglist = arglist;

    return n;
}

ast_id
ast_assign(
    struct ast**     astp,
    ast_id           identifier,
    ast_id           expr,
    struct utf8_span op_location,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_ASSIGNMENT, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        identifier > -1, log_parser_err("identifier: %d\n", identifier));
    ODBUTIL_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[identifier].info.node_type));

    ast->nodes[n].assignment.lvalue = identifier;
    ast->nodes[n].assignment.expr = expr;
    ast->nodes[n].assignment.op_location = op_location;

    return n;
}

ast_id
ast_identifier(
    struct ast**         astp,
    struct utf8_span     name,
    enum type_annotation annotation,
    struct utf8_span     location)
{
    ast_id      n = new_node(astp, AST_IDENTIFIER, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].identifier.name = name;
    ast->nodes[n].identifier.annotation = annotation;
    ast->nodes[n].identifier.explicit_type = TYPE_INVALID;
    ast->nodes[n].identifier.scope = SCOPE_LOCAL;

    return n;
}

ast_id
ast_binop(
    struct ast**     astp,
    enum binop_type  op,
    ast_id           left,
    ast_id           right,
    struct utf8_span op_location,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_BINOP, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(left > -1, log_parser_err("left: %d\n", left));
    ODBUTIL_DEBUG_ASSERT(right > -1, log_parser_err("right: %d\n", right));

    ast->nodes[n].binop.left = left;
    ast->nodes[n].binop.right = right;
    ast->nodes[n].binop.op_location = op_location;
    ast->nodes[n].binop.op = op;

    return n;
}

ast_id
ast_unop(
    struct ast**     astp,
    enum unop_type   op,
    ast_id           expr,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_UNOP, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));

    ast->nodes[n].unop.expr = expr;
    ast->nodes[n].unop.op = op;

    return n;
}

ast_id
ast_inc_step(
    struct ast** astp, ast_id var, ast_id expr, struct utf8_span location)
{
    ast_id add = ast_binop(astp, BINOP_ADD, var, expr, location, location);
    var = ast_dup_lvalue(astp, var);
    return ast_assign(astp, var, add, location, location);
}

ast_id
ast_inc(struct ast** astp, ast_id var, struct utf8_span location)
{
    ast_id expr = ast_byte_literal(astp, 1, location);
    return ast_inc_step(astp, var, expr, location);
}

ast_id
ast_dec_step(
    struct ast** astp, ast_id var, ast_id expr, struct utf8_span location)
{
    ast_id add = ast_binop(astp, BINOP_SUB, var, expr, location, location);
    var = ast_dup_lvalue(astp, var);
    return ast_assign(astp, var, add, location, location);
}

ast_id
ast_dec(struct ast** astp, ast_id var, struct utf8_span location)
{
    ast_id expr = ast_byte_literal(astp, 1, location);
    return ast_dec_step(astp, var, expr, location);
}

ast_id
ast_cond(
    struct ast**     astp,
    ast_id           expr,
    ast_id           cond_branch,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_COND, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ODBUTIL_DEBUG_ASSERT(
        cond_branch > -1, log_parser_err("cond_branch: %d\n", cond_branch));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[cond_branch].info.node_type == AST_COND_BRANCH,
        log_parser_err("type: %d\n", ast->nodes[cond_branch].info.node_type));

    ast->nodes[n].cond.expr = expr;
    ast->nodes[n].cond.cond_branch = cond_branch;
    return n;
}

ast_id
ast_cond_branch(
    struct ast** astp, ast_id yes, ast_id no, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_COND_BRANCH, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        yes == -1 || ast->nodes[yes].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[yes].info.node_type));
    ODBUTIL_DEBUG_ASSERT(
        no == -1 || ast->nodes[no].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[no].info.node_type));

    ast->nodes[n].cond_branch.yes = yes;
    ast->nodes[n].cond_branch.no = no;

    return n;
}

ast_id
ast_loop(
    struct ast**     astp,
    ast_id           body,
    struct utf8_span name,
    struct utf8_span implicit_name,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_LOOP, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].loop.body = body;
    ast->nodes[n].loop.name = name;
    ast->nodes[n].loop.loop_for = -1;
    ast->nodes[n].loop.implicit_name = implicit_name;

    return n;
}

ast_id
ast_loop_while(
    struct ast**     astp,
    ast_id           body,
    ast_id           expr,
    struct utf8_span name,
    struct utf8_span location)
{
    ast_id exit = ast_loop_exit(astp, empty_utf8_span(), location);
    ast_id exit_block = ast_block(astp, exit, location);
    ast_id cond_branch = ast_cond_branch(astp, -1, exit_block, location);
    ast_id cond = ast_cond(astp, expr, cond_branch, location);
    ast_id block = ast_block(astp, cond, location);

    if (body > -1)
        ast_block_append(*astp, block, body);

    return ast_loop(astp, block, name, empty_utf8_span(), location);
}

ast_id
ast_loop_until(
    struct ast**     astp,
    ast_id           body,
    ast_id           expr,
    struct utf8_span name,
    struct utf8_span location)
{
    ast_id exit = ast_loop_exit(astp, empty_utf8_span(), location);
    ast_id exit_block = ast_block(astp, exit, location);
    ast_id cond_branch = ast_cond_branch(astp, exit_block, -1, location);
    ast_id cond = ast_cond(astp, expr, cond_branch, location);

    if (body > -1)
        ast_block_append_stmt(astp, body, cond, location);
    else
        body = cond;

    return ast_loop(astp, body, name, empty_utf8_span(), location);
}

ast_id
ast_loop_for(
    struct ast**     astp,
    ast_id           body,
    ast_id           init,
    ast_id           end,
    ast_id           step,
    ast_id           next,
    struct utf8_span name,
    struct utf8_span location,
    const char*      filename,
    const char*      source)
{
    ast_id           loop, loop_for, loop_var;
    struct utf8_span implicit_name;
    struct ast*      ast = *astp;

    ODBUTIL_DEBUG_ASSERT(init > -1, log_parser_err("init: %d\n", init));
    ODBUTIL_DEBUG_ASSERT(end > -1, log_parser_err("init: %d\n", end));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[init].info.node_type == AST_ASSIGNMENT,
        log_parser_err("type: %d\n", ast->nodes[init].info.node_type));
    loop_var = ast->nodes[init].assignment.lvalue;
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[loop_var].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[loop_var].info.node_type));
    implicit_name = ast->nodes[loop_var].identifier.name;

    loop_for = new_node(astp, AST_LOOP_FOR, location);
    loop = ast_loop(astp, body, name, implicit_name, location);
    ast = *astp;
    if (loop < 0 || loop_for < 0)
        return -1;

    ast->nodes[loop].loop.loop_for = loop_for;
    ast->nodes[loop_for].loop_for.init = init;
    ast->nodes[loop_for].loop_for.end = end;
    ast->nodes[loop_for].loop_for.step = step;
    ast->nodes[loop_for].loop_for.next = next;

    return loop;
}

ast_id
ast_loop_cont(
    struct ast**     astp,
    struct utf8_span name,
    ast_id           step,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_LOOP_CONT, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].cont.name = name;
    ast->nodes[n].cont.step = step;

    return n;
}

ast_id
ast_loop_exit(
    struct ast** astp, struct utf8_span name, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_LOOP_EXIT, location);
    struct ast* ast = *astp;
    if (n < -1)
        return -1;

    ast->nodes[n].loop_exit.name = name;

    return n;
}

static int
ast_func_is_template(const struct ast* ast, int func)
{
    ast_id paramlist;
    ast_id decl;

    ODBUTIL_DEBUG_ASSERT(func > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[func].info.node_type == AST_FUNC,
        log_err("", "type: %d\n", ast->nodes[func].info.node_type));

    decl = ast->nodes[func].func.decl;
    for (paramlist = ast->nodes[decl].func_decl.paramlist; paramlist > -1;
         paramlist = ast->nodes[paramlist].paramlist.next)
    {
        ast_id identifier = ast->nodes[paramlist].paramlist.identifier;
        if (ast->nodes[identifier].identifier.explicit_type == TYPE_INVALID)
            return 1;
    }

    return 0;
}

ast_id
ast_func(
    struct ast**     astp,
    ast_id           identifier,
    ast_id           paramlist,
    ast_id           body,
    ast_id           retval,
    struct utf8_span location)
{
    struct ast* ast;
    ast_id      func, decl, def;
    func = new_node(astp, AST_FUNC, location);
    decl = new_node(astp, AST_FUNC_DECL, location);
    def = new_node(astp, AST_FUNC_DEF, location);
    ast = *astp;
    if (func < 0 || decl < 0 || def < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        identifier > -1, log_parser_err("identifier: %d\n", identifier));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[identifier].info.node_type));
    ast->nodes[decl].func_decl.identifier = identifier;

    ODBUTIL_DEBUG_ASSERT(
        paramlist == -1
            || ast->nodes[paramlist].info.node_type == AST_PARAMLIST,
        log_parser_err("type: %d\n", ast->nodes[paramlist].info.node_type));
    ast->nodes[decl].func_decl.paramlist = paramlist;

    ODBUTIL_DEBUG_ASSERT(
        body == -1 || ast->nodes[body].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[body].info.node_type));
    ast->nodes[def].func_def.body = body;
    ast->nodes[def].func_def.retval = retval;

    ast->nodes[func].func.decl = decl;
    ast->nodes[func].func.def = def;

    if (ast_func_is_template(ast, func))
        ast->nodes[func].info.node_type = AST_FUNC_TEMPLATE;

    return func;
}

ast_id
ast_func_or_container_ref(
    struct ast**     astp,
    ast_id           identifier,
    ast_id           arglist,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_FUNC_OR_CONTAINER_REF, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        identifier > -1, log_parser_err("identifier: %d\n", identifier));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[identifier].info.node_type));
    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast->nodes[arglist].info.node_type == AST_ARGLIST,
        log_parser_err("type: %d\n", ast->nodes[arglist].info.node_type));

    ast->nodes[n].func_or_container_ref.identifier = identifier;
    ast->nodes[n].func_or_container_ref.arglist = arglist;

    return n;
}

ast_id
ast_boolean_literal(struct ast** astp, char is_true, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_BOOLEAN_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].boolean_literal.is_true = is_true;

    return n;
}

ast_id
ast_byte_literal(struct ast** astp, uint8_t value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_BYTE_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].byte_literal.value = value;

    return n;
}
ast_id
ast_word_literal(struct ast** astp, uint16_t value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_WORD_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].word_literal.value = value;

    return n;
}
ast_id
ast_integer_literal(struct ast** astp, int32_t value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_INTEGER_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].integer_literal.value = value;

    return n;
}
ast_id
ast_dword_literal(struct ast** astp, uint32_t value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_DWORD_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].dword_literal.value = value;

    return n;
}
ast_id
ast_double_integer_literal(
    struct ast** astp, int64_t value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_DOUBLE_INTEGER_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].double_integer_literal.value = value;

    return n;
}
ast_id
ast_integer_like_literal(
    struct ast** astp, int64_t value, struct utf8_span location)
{
    if (value >= 0)
    {
        if (value > UINT32_MAX)
            return ast_double_integer_literal(astp, value, location);
        if (value > INT32_MAX)
            return ast_dword_literal(astp, (uint32_t)value, location);
        if (value > UINT16_MAX)
            return ast_integer_literal(astp, (int32_t)value, location);
        if (value > UINT8_MAX)
            return ast_word_literal(astp, (uint16_t)value, location);
        return ast_byte_literal(astp, (uint8_t)value, location);
    }

    if (value < INT32_MIN)
        return ast_double_integer_literal(astp, value, location);
    return ast_integer_literal(astp, (int32_t)value, location);
}

ast_id
ast_float_literal(struct ast** astp, float value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_FLOAT_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].float_literal.value = value;

    return n;
}
ast_id
ast_double_literal(struct ast** astp, double value, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_DOUBLE_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].double_literal.value = value;

    return n;
}

ast_id
ast_string_literal(
    struct ast** astp, struct utf8_span str, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_STRING_LITERAL, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[n].string_literal.str = str;

    return n;
}

ast_id
ast_cast(
    struct ast**     astp,
    ast_id           expr,
    enum type        target_type,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_CAST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));

    ast->nodes[n].cast.expr = expr;
    ast->nodes[n].info.type_info = target_type;

    return n;
}

ast_id
ast_scope(struct ast** astp, ast_id child, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_SCOPE, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(child > -1, log_parser_err("expr: %d\n", child));

    ast->nodes[n].scope.child = child;

    return n;
}
