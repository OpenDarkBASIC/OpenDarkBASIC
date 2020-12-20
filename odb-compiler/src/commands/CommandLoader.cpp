#include "odb-compiler/commands/CommandLoader.hpp"

namespace odb {
namespace cmd {

static std::vector<std::filesystem::path> stringsToPaths(const std::vector<std::string>& strings)
{
    std::vector<std::filesystem::path> paths;
    for (const auto& s : strings)
        paths.push_back(s);
    return paths;
}

// ----------------------------------------------------------------------------
CommandLoader::CommandLoader(const std::string& sdkRoot,
                             const std::vector<std::string>& pluginDirs) :
    sdkRoot_(sdkRoot),
    pluginDirs_(stringsToPaths(pluginDirs))
{
}

}
}
