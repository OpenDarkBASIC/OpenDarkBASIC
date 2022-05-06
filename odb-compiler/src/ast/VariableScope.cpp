#include "odb-compiler/ast/VariableScope.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
void VariableScope::add(ast::Variable* variable)
{
    variables_as_list_.emplace_back(variable);
    variables_[variable->name()][int(variable->annotation())] = variable;
}

// ----------------------------------------------------------------------------
ast::Variable* VariableScope::lookup(const std::string& name, ast::Annotation annotation) const
{
    auto it = variables_.find(name);
    if (it != variables_.end())
    {
        return it->second[int(annotation)];
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
const std::vector<ast::Variable*>& VariableScope::list() const
{
    return variables_as_list_;
}
}
