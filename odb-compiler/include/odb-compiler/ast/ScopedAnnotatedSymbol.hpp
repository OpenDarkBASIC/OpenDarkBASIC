#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Symbol.hpp"

namespace odb {
namespace ast {

class ScopedAnnotatedSymbol : public Symbol
{
public:
    ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location);
    Scope scope() const;
    Annotation annotation() const;

    void accept(Visitor* visitor) const override;

private:
    Scope scope_;
    Annotation annotation_;
};

}
}
