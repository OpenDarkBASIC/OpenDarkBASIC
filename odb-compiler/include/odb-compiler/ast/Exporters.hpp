#pragma once

#include "odb-compiler/config.hpp"
#include <cstdio>

namespace odb::ast {

class Node;

#if defined(ODBCOMPILER_DOT_EXPORT)
ODBCOMPILER_PUBLIC_API void dumpToDOT(FILE* fp, const Node* root);
#endif

#if defined(ODBCOMPILER_JSON_EXPORT)
ODBCOMPILER_PUBLIC_API void dumpToJSON(FILE* fp, const Node* root);
#endif

}
