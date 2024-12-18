#include "odb-compiler/ast/ast_convert.h"
#include "odb-util/log.h"

void
ast_convert_to_command(struct ast* ast, ast_id command_name, cmd_id cmd)
{
    union ast_node tmp = ast->nodes[command_name];
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, command_name) == AST_COMMAND_NAME,
        log_err("type: %d\n", ast_node_type(ast, command_name)));

    ast->nodes[command_name].info.node_type = AST_COMMAND;
    ast->nodes[command_name].command._pad = -1;
    ast->nodes[command_name].command.arglist = tmp.command_name.arglist;
    ast->nodes[command_name].command.is_expr = tmp.command_name.is_expr;
    ast->nodes[command_name].command.id = cmd;
}

void
ast_convert_to_func_call(struct ast* ast, ast_id call_like)
{
}

void
ast_convert_to_udt_init(
    struct ast* ast, ast_id udt_init, struct utf8_span type_name)
{
    union ast_node node = ast->nodes[udt_init];
    ast->nodes[udt_init].info.node_type = AST_UDT_INIT;
    ast->nodes[udt_init].udt_init.arglist = node.call_like.arglist;
    ast->nodes[udt_init].udt_init._pad = -1;
    ast->nodes[udt_init].udt_init.type_name = type_name;
}
