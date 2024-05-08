#pragma once

#include "odb-compiler/config.h"
#include "odb-sdk/utf8.h"

struct ast;
struct db_source;

#if defined(ODBCOMPILER_DOT_EXPORT)
int ast_export_dot(struct utf8_view filepath, const struct db_source* source, const struct ast* ast);
#endif

