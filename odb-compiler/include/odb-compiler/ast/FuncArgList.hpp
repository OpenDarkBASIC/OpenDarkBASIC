#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <vector>

namespace odb::ast {

class VarDecl;

class ODBCOMPILER_PUBLIC_API FuncArgList final : public Node
{
public:
    FuncArgList(SourceLocation* location);
    FuncArgList(VarDecl* initialVar, SourceLocation* location);

    void appendVarDecl(VarDecl* var);

    const std::vector<Reference<VarDecl>>& varDecls() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<VarDecl>> varDecls_;
};

}
