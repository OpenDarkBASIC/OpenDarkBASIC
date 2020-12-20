#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/commands/CommandLoader.hpp"

namespace odb {
namespace cmd {

class ODBCOMPILER_PUBLIC_API DBPCommandLoader : public CommandLoader
{
public:
    DBPCommandLoader(const std::string& sdkRoot,
                     const std::vector<std::string>& pluginDirs);

    bool populateIndex(CommandIndex* index) override;
    bool populateIndexFromLibrary(CommandIndex* index, DynamicLibrary* library) override;
};

}
}
