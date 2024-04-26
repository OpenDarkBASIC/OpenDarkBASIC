#include "odb-compiler/ast/VariableScope.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
void VariableScope::add(Variable* variable)
{
    variables_as_list_.emplace_back(variable);
    auto key = variableKey(variable->name(), variable->annotation(), variable->getType().isArray());
    variables_[key] = variable;
}

// ----------------------------------------------------------------------------
Variable* VariableScope::lookup(const std::string& name, Annotation annotation, bool isArray) const
{
    auto it = variables_.find(variableKey(name, annotation, isArray));
    if (it != variables_.end())
    {
        return it->second;
    }
    return nullptr;
}

// ----------------------------------------------------------------------------
const std::vector<Variable*>& VariableScope::list() const
{
    return variables_as_list_;
}

// ----------------------------------------------------------------------------
std::string VariableScope::variableKey(const std::string& name, Annotation annotation, bool isArray) const
{
    std::string key = name;
    if (annotation != Annotation::NONE)
    {
        key += typeAnnotationChar(annotation);
    }
    if (isArray)
    {
        key += "[";
    }
    return key;
}
}
