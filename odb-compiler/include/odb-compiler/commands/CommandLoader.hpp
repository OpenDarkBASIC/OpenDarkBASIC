#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/commands/SDKType.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace odb {
class TargetLibParser;

namespace cmd {
class CommandIndex;

class ODBCOMPILER_PUBLIC_API CommandLoader
{
public:
    CommandLoader(const std::filesystem::path& sdkRoot,
                  const std::vector<std::filesystem::path>& pluginDirs={});

    virtual ~CommandLoader() {}

    virtual bool populateIndex(CommandIndex* index) = 0;
    virtual bool populateIndexFromLibrary(CommandIndex* index, TargetLibParser* library) = 0;

protected:
    const std::filesystem::path sdkRoot_;
    const std::vector<std::filesystem::path> pluginDirs_;
};

}
}
