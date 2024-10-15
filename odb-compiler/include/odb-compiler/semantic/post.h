#pragma once

#include "odb-compiler/config.h"

struct ast;

ODBCOMPILER_PUBLIC_API void
post_delete_func_templates(struct ast** tus, int tu_count);

