#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Symbol.hpp"

namespace odb {
namespace ast {

class ScopedSymbol : public Symbol
{
public:
    ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location);
    Scope scope() const;

    void accept(Visitor* visitor) const override;

private:
    Scope scope_;
};

}
}
