#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/Program.hpp"
#include <vector>
#include <memory>

namespace odb {
namespace astpost {

class ODBCOMPILER_PUBLIC_API Process
{
public:
    virtual ~Process() = default;
    virtual bool execute(ast::Program* root) = 0;
};

class ODBCOMPILER_PUBLIC_API ProcessGroup
{
public:
    ProcessGroup() = default;
    ProcessGroup(const ProcessGroup&) = delete;
    ProcessGroup(ProcessGroup&&) = default;
    ProcessGroup& operator=(const ProcessGroup&) = delete;
    ProcessGroup& operator=(ProcessGroup&&) = default;

    void addProcess(std::unique_ptr<Process> process);
    bool execute(ast::Program* root);

private:
    std::vector<std::unique_ptr<Process>> processes_;
};

}
}
