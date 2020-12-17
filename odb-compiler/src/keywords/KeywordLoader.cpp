#include "odb-compiler/keywords/KeywordLoader.hpp"

namespace odb {
namespace kw {

static std::vector<std::filesystem::path> stringsToPaths(const std::vector<std::string>& strings)
{
    std::vector<std::filesystem::path> paths;
    for (const auto& s : strings)
        paths.push_back(s);
    return paths;
}

// ----------------------------------------------------------------------------
KeywordLoader::KeywordLoader(const std::string& sdkRoot,
                             const std::vector<std::string>& pluginDirs) :
    sdkRoot_(sdkRoot),
    pluginDirs_(stringsToPaths(pluginDirs))
{
}

}
}
