#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Variable.hpp"

#include <array>
#include <unordered_map>
#include <vector>

namespace odb::ast {

class VariableScope
{
public:
    void add(ast::Variable* variable);

    Variable* lookup(const std::string& name, Annotation annotation, bool isArray) const;
    const std::vector<Variable*>& list() const;

private:
    std::unordered_map<std::string, Reference<Variable>> variables_;
    std::vector<Variable*> variables_as_list_;

    std::string variableKey(const std::string& name, Annotation annotation, bool isArray) const;
};

}