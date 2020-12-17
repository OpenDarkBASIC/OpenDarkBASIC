#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <string>

namespace odb {
namespace ast {

class Symbol : public Node
{
public:
    enum class Annotation : char {
        NONE,
        STRING,
        FLOAT
    };

    enum class Scope : char {
        LOCAL,
        GLOBAL
    };

    Symbol(const std::string& name, SourceLocation* location);
    const std::string& name() const;

    void accept(Visitor* visitor) const override;

private:
    const std::string name_;
};

}
}
