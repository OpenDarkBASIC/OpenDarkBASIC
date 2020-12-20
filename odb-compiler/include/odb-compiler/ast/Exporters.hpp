#pragma once

#include "odb-compiler/config.hpp"
#include <cstdio>

namespace odb {
namespace ast {

class Node;

#if defined(ODBCOMPILER_DOT_EXPORT)
void dumpToDOT(FILE* fp, const Node* root);
#endif

}
}
