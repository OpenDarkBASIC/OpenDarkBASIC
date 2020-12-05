#include "odb-compiler/ast/Process.hpp"
#include "odb-compiler/ast/OldNode.hpp"
#include <cassert>

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Process::Process(Node* root) :
    root_(root)
{
}

// ----------------------------------------------------------------------------
bool Process::visitChildren(Node* node)
{

    return true;
}

}
}
