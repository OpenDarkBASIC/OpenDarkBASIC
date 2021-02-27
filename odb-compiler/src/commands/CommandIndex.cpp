#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/parsers/TargetLibParser.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/Str.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
void CommandIndex::addCommand(Command* command)
{
    commands_.emplace_back(command);
    commandLookupTable_.emplace(command->dbSymbol(), command);
}

// ----------------------------------------------------------------------------
bool CommandIndex::findConflicts() const
{
    std::unordered_map<std::string, std::vector<const Command*>> map_;

    for (const auto& cmd : commands_)
    {
        auto it = map_.insert({str::toLower(cmd->dbSymbol()), {cmd}});
        if (it.second)
            continue;

        // This dbSymbol already exists. Have to compare the new command with
        // all overloads of the existing symbol
        for (const auto& overload : it.first->second)
        {
            auto compare = [](const Command* a, const Command* b) -> bool {
                // Compare each argument type
                for (std::size_t i = 0; i != a->args().size() && i != b->args().size(); ++i)
                {
                    if (a->args()[i].type != b->args()[i].type)
                        return false;
                }
                if (a->args().size() != b->args().size())
                    return false;

                // Compare return type
                if (a->returnType() != b->returnType())
                    return false;

                return true;
            };

            if (compare(cmd, overload))
            {
                std::string typeinfo;
                typeinfo.push_back(static_cast<char>(cmd->returnType()));
                typeinfo.push_back('(');
                for (const auto& arg : cmd->args())
                    typeinfo.push_back(static_cast<char>(arg.type));
                typeinfo.push_back(')');

                Log::sdk(Log::ERROR, "Command `%s %s` redefined in library `%s`\n", cmd->dbSymbol().c_str(), typeinfo.c_str(), cmd->library()->getFilename());
                Log::sdk(Log::NOTICE, "Command was first declared in library `%s`\n", overload->library()->getFilename());
                return true;
            }
        }
    }

    return false;
}

// ----------------------------------------------------------------------------
std::vector<Reference<Command>> CommandIndex::lookup(std::string commandName) const
{
    std::transform(commandName.begin(), commandName.end(), commandName.begin(),
                   [](char c) { return std::tolower(c); });
    std::vector<Reference<Command>> matches;
    auto overloadIteratorRange = commandLookupTable_.equal_range(commandName);
    for (auto match = overloadIteratorRange.first; match != overloadIteratorRange.second; ++match) {
        matches.emplace_back(match->second);
    }
    return matches;
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Command>>& CommandIndex::commands() const
{
    return commands_;
}

// ----------------------------------------------------------------------------
std::vector<std::string> CommandIndex::commandNamesAsList() const
{
    std::vector<std::string> list;
    list.reserve(commands_.size());
    for (const auto& cmd : commands_)
        list.push_back(cmd->dbSymbol());
    return list;
}

// ----------------------------------------------------------------------------
std::vector<TargetLibParser*> CommandIndex::librariesAsList() const
{
    std::unordered_set<TargetLibParser*> librarySet;
    librarySet.reserve(commands_.size());
    for (const auto& cmd : commands_)
        librarySet.emplace(cmd->library());
    return std::vector<TargetLibParser*>(librarySet.begin(), librarySet.end());
}

}
}
