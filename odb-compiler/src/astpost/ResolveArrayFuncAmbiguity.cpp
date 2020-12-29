#include "odb-compiler/astpost/ResolveArrayFuncAmbiguity.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-sdk/Log.hpp"
#include <unordered_map>
#include <string>

namespace odb {
namespace astpost {

class Gatherer : public ast::GenericVisitor
{
public:
    void visit(ast::Node* node) override final { /* don't care */ }

private:
    std::unordered_map<std::string, const ast::Node*> arraySymbols_;
};

// ----------------------------------------------------------------------------
bool ResolveArrayFuncAmbiguity::execute(ast::Node* node)
{
    return false;
}

}
}
