#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Symbol.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol : public Symbol
{
public:

    AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location);
    Annotation annotation() const;

    void accept(Visitor* visitor) const override;

private:
    Annotation annotation_;
};

}
}
