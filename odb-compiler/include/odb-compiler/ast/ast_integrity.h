#pragma once

#include "odb-compiler/config.h"

struct ast;

ODBCOMPILER_PUBLIC_API int
ast_verify_connectivity(const struct ast* ast);

