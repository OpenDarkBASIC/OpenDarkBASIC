#pragma once

#include "odb-compiler/parser/db_parser.y.h"
#include "odb-util/utf8.h"

dbtoken_kind_t
db_keyword_lookup(const char* data, struct utf8_span ref);
