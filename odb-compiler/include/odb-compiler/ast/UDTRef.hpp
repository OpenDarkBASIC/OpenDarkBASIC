#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Symbol.hpp"

namespace odb::ast {

class UDTRef : public Symbol
{
public:
    UDTRef(const std::string& name, SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;

protected:
    Node* duplicateImpl() const override;
};

}
