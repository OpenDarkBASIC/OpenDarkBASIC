#pragma once

#include <string>
#include <vector>

namespace odb::cmd {
    class CommandIndex;
}

bool loadCommands(const std::vector<std::string>& args);
bool dumpCommandsJSON(const std::vector<std::string>& args);
bool dumpCommandsINI(const std::vector<std::string> &args);
bool dumpCommandNames(const std::vector<std::string>& args);

const odb::cmd::CommandIndex* getCommandIndex();
