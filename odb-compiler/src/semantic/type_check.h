#pragma once

#include "odb-compiler/ast/ast.h"

enum type
type_check_binop_symmetric(
    struct ast** astp, ast_id op, const char* filename, const char* source);

enum type
type_check_binop_pow(
    struct ast** astp, ast_id op, const char* filename, const char* source);

enum type
type_check_casts(
    struct ast* ast, ast_id cast, const char* filename, const char* source);
