#include "odb-compiler/astpost/Process.hpp"

namespace odb::astpost {

// ----------------------------------------------------------------------------
void ProcessGroup::addProcess(std::unique_ptr<Process> process)
{
    processes_.push_back(std::move(process));
}

// ----------------------------------------------------------------------------
bool ProcessGroup::execute(ast::Node* root)
{
    for (const auto& process : processes_)
        if (process->execute(root) == false)
            return false;

    return true;
}

}
