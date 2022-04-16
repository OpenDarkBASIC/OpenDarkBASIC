#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Symbol.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API UDTRef final : public Symbol
{
public:
    UDTRef(const std::string& name, SourceLocation* location);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;

protected:
    Node* duplicateImpl() const override;
};

}
