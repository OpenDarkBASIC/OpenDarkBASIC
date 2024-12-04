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

    ast->nodes[n].info.location = location;
    ast->nodes[n].info.scope_id = -1;
    ast->nodes[n].info.node_type = type;
    ast->nodes[n].info.type_info = primitive_type(TYPE_INVALID);
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
ast_dup_node_into(struct ast** dst_astp, const struct ast* src_ast, ast_id n)
{
    ast_id dup = ast_grow(dst_astp);
    if (dup < 0)
        return -1;

    memcpy(
        &(*dst_astp)->nodes[dup], &src_ast->nodes[n], sizeof(union ast_node));
    return dup;
}

ast_id
ast_block(struct ast** astp, ast_id stmt, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_BLOCK, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(stmt > -1, (void)0);
    ast->nodes[n].block.stmt = stmt;

    return n;
}

void
ast_block_append(struct ast* ast, ast_id block, ast_id append_block)
{
    ODBUTIL_DEBUG_ASSERT(block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, block)));
    ODBUTIL_DEBUG_ASSERT(append_block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, append_block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, append_block)));

    while (ast->nodes[block].block.next > -1)
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

    ODBUTIL_DEBUG_ASSERT(block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(stmt > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, block)));

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

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ast->nodes[n].arglist.expr = expr;
    ast->nodes[n].arglist.combined_location = location;

    return n;
}

ast_id
ast_arglist_append_expr(
    struct ast** astp, ast_id arglist, ast_id expr, struct utf8_span location)
{
    struct utf8_span combined_location;
    ast_id           n = new_node(astp, AST_ARGLIST, location);
    struct ast*      ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(arglist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    combined_location
        = utf8_span_union(ast->nodes[arglist].info.location, location);
    while (ast->nodes[arglist].arglist.next != -1)
    {
        ast->nodes[arglist].arglist.combined_location = combined_location;
        arglist = ast->nodes[arglist].arglist.next;
    }

    ast->nodes[arglist].arglist.combined_location = combined_location;
    ast->nodes[arglist].arglist.next = n;
    ast->nodes[n].arglist.expr = expr;
    ast->nodes[n].arglist.combined_location = combined_location;

    return n;
}

ast_id
ast_paramlist(struct ast** astp, ast_id param, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_PARAMLIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));

    ast->nodes[n].paramlist.param = param;
    ast->nodes[n].paramlist.combined_location = location;

    return n;
}

ast_id
ast_paramlist_append(
    struct ast**     astp,
    ast_id           paramlist,
    ast_id           param,
    struct utf8_span location)
{
    struct utf8_span combined_location;
    ast_id           n = new_node(astp, AST_PARAMLIST, location);
    struct ast*      ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(paramlist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, paramlist) == AST_PARAMLIST, (void)0);

    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));

    combined_location
        = utf8_span_union(ast->nodes[paramlist].info.location, location);
    for (; ast->nodes[paramlist].paramlist.next > -1;
         paramlist = ast->nodes[paramlist].paramlist.next)
        ast->nodes[paramlist].paramlist.combined_location = combined_location;

    ast->nodes[paramlist].paramlist.combined_location = combined_location;
    ast->nodes[paramlist].paramlist.next = n;
    ast->nodes[n].paramlist.param = param;
    ast->nodes[n].paramlist.combined_location = combined_location;

    return n;
}

ast_id
ast_typelist(
    struct ast**     astp,
    struct utf8_span name,
    ast_id           type,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_TYPELIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(type > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, type) == AST_AS_TYPE
            || ast_node_type(ast, type) == AST_AS_EXPR
            || ast_node_type(ast, type) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(ast, type)));

    ast->nodes[n].typelist.type = type;
    ast->nodes[n].typelist.name = name;

    return n;
}

void
ast_typelist_append(
    struct ast*      ast,
    ast_id           typelist,
    ast_id           append_typelist,
    struct utf8_span location)
{
    ODBUTIL_DEBUG_ASSERT(typelist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, typelist) == AST_TYPELIST,
        log_err("type: %d\n", ast_node_type(ast, typelist)));
    ODBUTIL_DEBUG_ASSERT(append_typelist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, append_typelist) == AST_TYPELIST,
        log_err("type: %d\n", ast_node_type(ast, append_typelist)));

    while (ast->nodes[typelist].typelist.next > -1)
        typelist = ast->nodes[typelist].typelist.next;

    ast->nodes[typelist].typelist.next = append_typelist;
}

ast_id
ast_typelist_append_type(
    struct ast** astp, ast_id typelist, ast_id type, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_TYPELIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(typelist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(type > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, typelist) == AST_TYPELIST,
        log_err("type: %d\n", ast_node_type(ast, typelist)));

    ast_typelist_append(ast, typelist, n, location);
    ast->nodes[n].typelist.type = type;

    return n;
}

ast_id
ast_load_plugin(
    struct ast** astp, struct utf8_span filepath, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_LOAD_PLUGIN, location);
    if (n < 0)
        return -1;

    (*astp)->nodes[n].load_plugin.filepath = filepath;

    return n;
}

ast_id
ast_load_command(
    struct ast**     astp,
    ast_id           typelist,
    ast_id           rettype,
    struct utf8_span cmd_name,
    struct utf8_span filepath,
    struct utf8_span c_symbol,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_LOAD_COMMAND, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, rettype) == AST_AS_TYPE
            || ast_node_type(ast, rettype) == AST_AS_EXPR
            || ast_node_type(ast, rettype) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(ast, rettype)));
    ODBUTIL_DEBUG_ASSERT(
        typelist == -1 || ast_node_type(ast, typelist) == AST_TYPELIST,
        log_err("type: %d\n", ast_node_type(ast, typelist)));

    ast->nodes[n].load_command.cmd_name = cmd_name;
    ast->nodes[n].load_command.filepath = filepath;
    ast->nodes[n].load_command.c_symbol = c_symbol;
    ast->nodes[n].load_command.rettype = rettype;
    ast->nodes[n].load_command.typelist = typelist;

    return n;
}

ast_id
ast_command_name(
    struct ast**     astp,
    struct utf8_span command_name,
    ast_id           arglist,
    char             is_expr,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_COMMAND_NAME, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    ast->nodes[n].command_name.arglist = arglist;
    ast->nodes[n].command_name.name = command_name;
    ast->nodes[n].command_name.is_expr = !!is_expr;

    return n;
}

ast_id
ast_assign(
    struct ast**     astp,
    ast_id           lvalue,
    ast_id           expr,
    struct utf8_span op_location,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_ASSIGNMENT, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(lvalue > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, lvalue) == AST_VAR_DECL1
            || ast_node_type(ast, lvalue) == AST_VAR_WRITE
            || ast_node_type(ast, lvalue) == AST_CONTAINER_WRITE
            || ast_node_type(ast, lvalue) == AST_UDT_WRITE,
        log_err("type: %d\n", ast_node_type(ast, lvalue)));

    ast->nodes[n].assignment.lvalue = lvalue;
    ast->nodes[n].assignment.expr = expr;
    ast->nodes[n].assignment.op_location = op_location;

    return n;
}

ast_id
ast_var_decl(
    struct ast**     astp,
    ast_id           identifier,
    ast_id           as,
    ast_id           init_expr,
    enum scope       scope,
    struct utf8_span scope_location,
    struct utf8_span op_location,
    struct utf8_span location)
{
    ast_id decl1 = new_node(astp, AST_VAR_DECL1, location);
    ast_id decl2 = new_node(astp, AST_VAR_DECL2, location);
    if (decl1 < 0 || decl2 < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    ODBUTIL_DEBUG_ASSERT(
        as == -1 || ast_node_type(*astp, as) == AST_AS_TYPE
            || ast_node_type(*astp, as) == AST_AS_EXPR
            || ast_node_type(*astp, as) == AST_AS_AUTO
            || ast_node_type(*astp, as) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(*astp, as)));

    (*astp)->nodes[decl1].var_decl1.init_expr = init_expr;
    (*astp)->nodes[decl1].var_decl1.scope = scope;
    (*astp)->nodes[decl1].var_decl1.scope_location = scope_location;
    (*astp)->nodes[decl1].var_decl1.var_decl2 = decl2;

    (*astp)->nodes[decl2].var_decl2.identifier = identifier;
    (*astp)->nodes[decl2].var_decl2.as = as;
    (*astp)->nodes[decl2].var_decl2.op_location = op_location;

    return decl1;
}

ast_id
ast_var_read(struct ast** astp, ast_id identifier, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_VAR_READ, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    (*astp)->nodes[n].var_read.identifier = identifier;

    return n;
}

ast_id
ast_var_write(struct ast** astp, ast_id identifier, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_VAR_WRITE, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, identifier)));

    (*astp)->nodes[n].var_write.identifier = identifier;

    return n;
}

ast_id
ast_udt_decl(
    struct ast**     astp,
    ast_id           type_identifier,
    ast_id           members_block,
    struct utf8_span location)
{
    ast_id n = new_node(astp, AST_UDT_DECL, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, type_identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(*astp, type_identifier)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, members_block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(*astp, members_block)));

    (*astp)->nodes[n].udt_decl.type_identifier = type_identifier;
    (*astp)->nodes[n].udt_decl.members = members_block;

    return n;
}

ast_id
ast_udt_init(
    struct ast**     astp,
    struct utf8_span type_name,
    ast_id           arglist,
    struct utf8_span location)
{
    ast_id n = new_node(astp, AST_UDT_INIT, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(*astp, arglist)));

    (*astp)->nodes[n].udt_init.type_name = type_name;
    (*astp)->nodes[n].udt_init.arglist = arglist;

    return n;
}

ast_id
ast_udt_read(
    struct ast** astp, ast_id member, ast_id next, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_UDT_READ, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(member > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(next > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, member) == AST_VAR_READ
            || ast_node_type(*astp, member) == AST_CALL_LIKE,
        log_err("type: %d\n", ast_node_type(*astp, member)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, member) == AST_UDT_READ
            || ast_node_type(*astp, member) == AST_VAR_READ
            || ast_node_type(*astp, member) == AST_CALL_LIKE,
        log_err("type: %d\n", ast_node_type(*astp, next)));

    (*astp)->nodes[n].udt_read.left = member;
    (*astp)->nodes[n].udt_read.right = next;
    (*astp)->nodes[n].udt_read.index = -1;

    return n;
}

ast_id
ast_param(
    struct ast** astp, ast_id identifier, ast_id as, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_PARAM, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        as == -1 || ast_node_type(ast, as) == AST_AS_TYPE
            || ast_node_type(ast, as) == AST_AS_EXPR
            || ast_node_type(ast, as) == AST_AS_AUTO
            || ast_node_type(ast, as) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(ast, as)));

    ast->nodes[n].param.identifier = identifier;
    ast->nodes[n].param.as = as;

    return n;
}

ast_id
ast_udt_write(
    struct ast** astp, ast_id member, ast_id next, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_UDT_WRITE, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(member > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(next > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, member) == AST_VAR_WRITE
            || ast_node_type(*astp, member) == AST_CONTAINER_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, member)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, member) == AST_UDT_WRITE
            || ast_node_type(*astp, member) == AST_VAR_WRITE
            || ast_node_type(*astp, member) == AST_CONTAINER_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, next)));

    (*astp)->nodes[n].udt_write.left = member;
    (*astp)->nodes[n].udt_write.right = next;
    (*astp)->nodes[n].udt_write.index = -1;

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

    ODBUTIL_DEBUG_ASSERT(left > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(right > -1, (void)0);

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

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);

    ast->nodes[n].unop.expr = expr;
    ast->nodes[n].unop.op = op;

    return n;
}

static void
convert_lvalue_to_rvalue(struct ast* ast, ast_id lvalue)
{
    if (ast_node_type(ast, lvalue) == AST_VAR_WRITE)
    {
        union ast_node node = ast->nodes[lvalue];
        node.info.node_type = AST_VAR_READ;
        node.var_read.identifier = ast->nodes[lvalue].var_write.identifier;
        node.var_read._pad = -1;
        ast->nodes[lvalue] = node;
    }
    else if (ast_node_type(ast, lvalue) == AST_UDT_WRITE)
    {
        union ast_node node = ast->nodes[lvalue];
        node.info.node_type = AST_UDT_READ;
        node.udt_write.left = ast->nodes[lvalue].udt_read.left;
        node.udt_write.right = ast->nodes[lvalue].udt_read.right;
        node.udt_write.index = ast->nodes[lvalue].udt_read.index;
        ast->nodes[lvalue] = node;

        convert_lvalue_to_rvalue(ast, ast->nodes[lvalue].udt_write.left);
        convert_lvalue_to_rvalue(ast, ast->nodes[lvalue].udt_write.right);
    }
    else
    {
        ODBUTIL_DEBUG_ASSERT(
            0, log_err("type: %d\n", ast_node_type(ast, lvalue)));
    }
}

ast_id
ast_inc_step(
    struct ast** astp, ast_id lvalue, ast_id expr, struct utf8_span location)
{
    ast_id add, rvalue;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, lvalue) == AST_VAR_WRITE
            || ast_node_type(*astp, lvalue) == AST_UDT_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, lvalue)));

    rvalue = ast_dup_subtree(astp, lvalue);
    if (rvalue < 0)
        return -1;
    convert_lvalue_to_rvalue(*astp, rvalue);

    add = ast_binop(astp, BINOP_ADD, rvalue, expr, location, location);
    if (add < 0)
        return -1;

    return ast_assign(astp, lvalue, add, location, location);
}

ast_id
ast_inc(struct ast** astp, ast_id lvalue, struct utf8_span location)
{
    ast_id expr = ast_byte_literal(astp, 1, location);
    if (expr < 0)
        return -1;

    return ast_inc_step(astp, lvalue, expr, location);
}

ast_id
ast_dec_step(
    struct ast** astp, ast_id lvalue, ast_id expr, struct utf8_span location)
{
    ast_id sub, rvalue;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, lvalue) == AST_VAR_WRITE
            || ast_node_type(*astp, lvalue) == AST_UDT_WRITE,
        log_err("type: %d\n", ast_node_type(*astp, lvalue)));

    rvalue = ast_dup_subtree(astp, lvalue);
    if (rvalue < 0)
        return -1;
    convert_lvalue_to_rvalue(*astp, rvalue);

    sub = ast_binop(astp, BINOP_SUB, rvalue, expr, location, location);
    if (sub < 0)
        return -1;

    return ast_assign(astp, lvalue, sub, location, location);
}

ast_id
ast_dec(struct ast** astp, ast_id var_write, struct utf8_span location)
{
    ast_id expr = ast_byte_literal(astp, 1, location);
    if (expr < 0)
        return -1;

    return ast_dec_step(astp, var_write, expr, location);
}

ast_id
ast_cond(
    struct ast**     astp,
    ast_id           expr,
    ast_id           cond_branches,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_COND, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(cond_branches > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cond_branches) == AST_COND_BRANCHES,
        log_err("type: %d\n", ast_node_type(ast, cond_branches)));

    ast->nodes[n].cond.expr = expr;
    ast->nodes[n].cond.cond_branches = cond_branches;
    return n;
}

ast_id
ast_cond_branches(
    struct ast** astp, ast_id yes, ast_id no, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_COND_BRANCHES, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        yes == -1 || ast_node_type(ast, yes) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, yes)));
    ODBUTIL_DEBUG_ASSERT(
        no == -1 || ast_node_type(ast, no) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, no)));

    ast->nodes[n].cond_branches.yes = yes;
    ast->nodes[n].cond_branches.no = no;

    return n;
}

ast_id
ast_select(
    struct ast** astp, ast_id expr, ast_id caselist, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_SELECT, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        caselist == -1 || ast_node_type(ast, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(ast, caselist)));

    ast->nodes[n].select.expr = expr;
    ast->nodes[n].select.caselist = caselist;

    return n;
}

ast_id
ast_caselist(struct ast** astp, ast_id case_, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_CASELIST, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(case_ > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(*astp, case_)));

    (*astp)->nodes[n].caselist.case_ = case_;

    return n;
}

void
ast_caselist_append(
    struct ast*      ast,
    ast_id           caselist,
    ast_id           append_caselist,
    struct utf8_span location)
{
    ODBUTIL_DEBUG_ASSERT(caselist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(ast, caselist)));
    ODBUTIL_DEBUG_ASSERT(append_caselist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, append_caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(ast, append_caselist)));

    while (ast->nodes[caselist].caselist.next > -1)
        caselist = ast->nodes[caselist].caselist.next;

    ast->nodes[caselist].caselist.next = append_caselist;
}

ast_id
ast_caselist_append_case(
    struct ast** astp, ast_id caselist, ast_id case_, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_CASELIST, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(caselist > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(ast, caselist)));
    ODBUTIL_DEBUG_ASSERT(case_ > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, case_)));

    ast_caselist_append(ast, caselist, n, location);
    ast->nodes[n].caselist.case_ = case_;

    return n;
}

ast_id
ast_case(
    struct ast**     astp,
    ast_id           expr,
    ast_id           body,
    struct utf8_span case_loc,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_CASE, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(
        body == -1 || ast_node_type(ast, body) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, body)));

    ast->nodes[n].case_.expr = expr;
    ast->nodes[n].case_.body = body;
    ast->nodes[n].case_.case_loc = case_loc;

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
    ast_id      n = new_node(astp, AST_LOOP1, location);
    ast_id      loop_body = new_node(astp, AST_LOOP2, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ast->nodes[loop_body].loop2.body = body;

    ast->nodes[n].loop1.loop2 = loop_body;
    ast->nodes[n].loop1.name = name;
    ast->nodes[n].loop1.implicit_name = implicit_name;

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
    ast_id cond_branches = ast_cond_branches(astp, -1, exit_block, location);
    ast_id cond = ast_cond(astp, expr, cond_branches, location);
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
    ast_id cond_branches = ast_cond_branches(astp, exit_block, -1, location);
    ast_id cond = ast_cond(astp, expr, cond_branches, location);

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
    struct utf8_span location)
{
    ast_id           loop, loop_for1, loop_for2, loop_for3;
    struct utf8_span implicit_name;
    struct ast*      ast = *astp;

    ODBUTIL_DEBUG_ASSERT(init > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(end > -1, (void)0);

    /* Init expression can be an assignment (includes array/udt refs), but it
     * also makes an exception to allow for variable declarations with AS TYPE
     */
    switch (ast_node_type(ast, init))
    {
        case AST_ASSIGNMENT: {
            ast_id lvalue = ast->nodes[init].assignment.lvalue;
            ast_id identifier = ast->nodes[lvalue].var_write.identifier;
            implicit_name = ast->nodes[identifier].identifier.name;
            break;
        }
        case AST_VAR_DECL1: {
            ast_id decl2 = ast->nodes[init].var_decl1.var_decl2;
            ast_id identifier = ast->nodes[decl2].var_decl2.identifier;
            implicit_name = ast->nodes[identifier].identifier.name;
            break;
        }
        default:
            ODBUTIL_DEBUG_ASSERT(
                0, log_err("type: %d\n", ast_node_type(ast, init)));
            return -1;
    }

    loop_for1 = new_node(astp, AST_LOOP_FOR1, location);
    loop_for2 = new_node(astp, AST_LOOP_FOR2, location);
    loop_for3 = new_node(astp, AST_LOOP_FOR3, location);
    loop = ast_loop(astp, body, name, implicit_name, location);
    ast = *astp;
    if (loop < 0 || loop_for1 < 0 || loop_for2 < 0 || loop_for3 < 0)
        return -1;

    ast->nodes[loop_for3].loop_for3.step = step;
    ast->nodes[loop_for3].loop_for3.next = next;

    ast->nodes[loop_for2].loop_for2.loop_for3 = loop_for3;
    ast->nodes[loop_for2].loop_for2.end = end;

    ast->nodes[loop_for1].loop_for1.loop_for2 = loop_for2;
    ast->nodes[loop_for1].loop_for1.init = init;

    ast->nodes[loop].loop1.loop_for1 = loop_for1;

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
ast_func_is_polymorphic(const struct ast* ast, ast_id paramlist)
{
    ODBUTIL_DEBUG_ASSERT(
        paramlist == -1 || ast_node_type(ast, paramlist) == AST_PARAMLIST,
        log_err("type: %d\n", ast_node_type(ast, paramlist)));

    for (; paramlist > -1; paramlist = ast->nodes[paramlist].paramlist.next)
    {
        ast_id param = ast->nodes[paramlist].paramlist.param;
        if (ast->nodes[param].param.as == -1)
            return 1;
    }

    return 0;
}

ast_id
ast_func(
    struct ast**     astp,
    enum scope       scope,
    ast_id           identifier,
    ast_id           as,
    ast_id           paramlist,
    ast_id           body,
    ast_id           retval,
    struct utf8_span endfunction_location,
    struct utf8_span location)
{
    struct ast* ast;
    ast_id      f1, f2, f3, f4;
    f1 = new_node(astp, AST_FUNC1, location);
    f2 = new_node(astp, AST_FUNC2, location);
    f3 = new_node(astp, AST_FUNC3, location);
    f4 = new_node(astp, AST_FUNC4, location);
    ast = *astp;
    if (f1 < 0 || f2 < 0 || f3 < 0 || f4 < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    ODBUTIL_DEBUG_ASSERT(
        as == -1 || ast_node_type(*astp, as) == AST_AS_TYPE
            || ast_node_type(*astp, as) == AST_AS_EXPR
            || ast_node_type(*astp, as) == AST_AS_AUTO
            || ast_node_type(*astp, as) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(*astp, as)));

    ODBUTIL_DEBUG_ASSERT(
        paramlist == -1 || ast_node_type(ast, paramlist) == AST_PARAMLIST,
        log_err("type: %d\n", ast_node_type(ast, paramlist)));

    ODBUTIL_DEBUG_ASSERT(
        body == -1 || ast_node_type(ast, body) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, body)));

    ast->nodes[f1].func1.identifier = identifier;
    ast->nodes[f1].func1.func2 = f2;
    ast->nodes[f1].func1.endfunction_location = endfunction_location;
    ast->nodes[f1].func1.scope = scope;

    ast->nodes[f2].func2.as = as;
    ast->nodes[f2].func2.func3 = f3;

    ast->nodes[f3].func3.paramlist = paramlist;
    ast->nodes[f3].func3.func4 = f4;

    ast->nodes[f4].func4.body = body;
    ast->nodes[f4].func4.retval = retval;

    if (ast_func_is_polymorphic(ast, paramlist))
    {
        ast_id poly = new_node(astp, AST_FUNC_POLY, location);
        if (poly < 0)
            return -1;
        ast->nodes[poly].func_poly.func = f1;
        return poly;
    }

    return f1;
}

ast_id
ast_func_exit(struct ast** astp, ast_id retval, struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_FUNC_EXIT, location);
    struct ast* ast = *astp;
    if (n < -1)
        return -1;

    ast->nodes[n].func_exit.retval = retval;

    return n;
}

ast_id
ast_call_like(
    struct ast**     astp,
    ast_id           identifier,
    ast_id           arglist,
    char             is_expr,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_CALL_LIKE, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    ast->nodes[n].call_like.identifier = identifier;
    ast->nodes[n].call_like.arglist = arglist;
    ast->nodes[n].call_like.is_expr = !!is_expr;

    return n;
}

ast_id
ast_container_write(
    struct ast**     astp,
    ast_id           identifier,
    ast_id           arglist,
    struct utf8_span location)
{
    ast_id      n = new_node(astp, AST_CONTAINER_WRITE, location);
    struct ast* ast = *astp;
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        arglist == -1 || ast_node_type(ast, arglist) == AST_ARGLIST,
        log_err("type: %d\n", ast_node_type(ast, arglist)));

    ast->nodes[n].container_write.identifier = identifier;
    ast->nodes[n].container_write.arglist = arglist;

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
ast_cast(struct ast** astp, ast_id expr, ast_id as, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_CAST, location);
    if (n < 0)
        return -1;

    ODBUTIL_DEBUG_ASSERT(expr > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(as > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        as == -1 || ast_node_type(*astp, as) == AST_AS_TYPE
            || ast_node_type(*astp, as) == AST_AS_EXPR
            || ast_node_type(*astp, as) == AST_AS_AUTO
            || ast_node_type(*astp, as) == AST_AS_UDT,
        log_err("type: %d\n", ast_node_type(*astp, as)));

    (*astp)->nodes[n].cast.expr = expr;
    (*astp)->nodes[n].cast.as = as;

    return n;
}

ast_id
ast_cast_to_primitive_type(
    struct ast**        astp,
    ast_id              expr,
    enum primitive_type target_type,
    struct utf8_span    location)
{
    ast_id as = ast_as_type(astp, primitive_type(target_type), location);
    if (as < 0)
        return -1;

    return ast_cast(astp, expr, as, location);
}

ast_id
ast_as_type(
    struct ast** astp, union type target_type, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_AS_TYPE, location);
    if (n < 0)
        return -1;

    (*astp)->nodes[n].as_type.type = target_type;

    return n;
}

ast_id
ast_as_expr(struct ast** astp, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_AS_EXPR, location);
    if (n < 0)
        return -1;

    (*astp)->nodes[n].as_expr.expr = expr;

    return n;
}

ast_id
ast_as_auto(struct ast** astp, struct utf8_span location)
{
    return new_node(astp, AST_AS_AUTO, location);
}

ast_id
ast_as_udt(
    struct ast** astp, struct utf8_span type_name, struct utf8_span location)
{
    ast_id n = new_node(astp, AST_AS_UDT, location);
    if (n < 0)
        return -1;

    (*astp)->nodes[n].as_udt.type_name = type_name;

    return n;
}
