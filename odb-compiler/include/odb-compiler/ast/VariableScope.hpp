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

    ast::Variable* lookup(const std::string& name, ast::Annotation annotation) const;
    const std::vector<ast::Variable*>& list() const;

private:
    std::unordered_map<std::string, std::array<Reference<ast::Variable>, 6>> variables_;
    std::vector<ast::Variable*> variables_as_list_;
};

}