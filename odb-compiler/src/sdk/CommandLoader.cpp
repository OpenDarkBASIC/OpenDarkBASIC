#include "odb-compiler/commands/CommandLoader.hpp"

namespace odb {
namespace cmd {

// ----------------------------------------------------------------------------
CommandLoader::CommandLoader(const std::filesystem::path& sdkRoot,
                             const std::vector<std::filesystem::path>& pluginDirs) :
    sdkRoot_(sdkRoot),
    pluginDirs_(pluginDirs)
{
}

}
}
