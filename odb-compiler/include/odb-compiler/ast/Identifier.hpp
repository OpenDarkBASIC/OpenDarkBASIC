#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API Identifier : public Node
{
public:
    Identifier(Program* program, SourceLocation* location, std::string name);
    Identifier(Program* program, SourceLocation* location, std::string name, Annotation annotation);

    const std::string& name() const;
    Annotation annotation() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

protected:
    const std::string name_;
    Annotation annotation_;
};

}
