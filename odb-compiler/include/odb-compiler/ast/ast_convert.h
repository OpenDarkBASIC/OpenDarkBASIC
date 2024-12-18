#pragma once

#include "odb-compiler/ast/ast.h"

void ast_convert_to_command(struct ast* ast, ast_id command_name, cmd_id cmd);
void ast_convert_to_func_call(struct ast* ast, ast_id call_like);
void ast_convert_to_udt_init(struct ast* ast, ast_id udt_init, struct utf8_span type_name);
