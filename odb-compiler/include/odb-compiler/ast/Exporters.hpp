#pragma once

#include "odb-compiler/config.hpp"
#include <cstdio>

namespace odb {
namespace ast {

class Node;

#if defined(ODBCOMPILER_DOT_EXPORT)
ODBCOMPILER_PUBLIC_API void dumpToDOT(FILE* fp, const Node* root);
#endif

}
}
