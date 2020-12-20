#include "odb-compiler/astpost/ErrorHighlighter.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ErrorHighlighter::ErrorHighlighter(Node* root) :
    Process(root)
{
}

// ----------------------------------------------------------------------------
bool ErrorHighlighter::execute()
{
    return false;
}

// ----------------------------------------------------------------------------
bool ErrorHighlighter::processNode(Node* node)
{
    return false;
}

}
}
