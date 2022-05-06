#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Type.hpp"
#include "odb-compiler/ast/LValue.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API Variable final : public LValue
{
public:
    Variable(SourceLocation* location, std::string name, Type type);
    Variable(SourceLocation* location, std::string name, Annotation annotation, Type type);

    const std::string& name() const;
    Annotation annotation() const;
    Type getType() const override;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

    const std::string name_;
    Annotation annotation_;
    Type type_;
};

}
