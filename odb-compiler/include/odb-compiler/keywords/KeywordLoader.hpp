#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include <filesystem>
#include <vector>

namespace odb {

class DynamicLibrary;
class KeywordIndex;

class KeywordLoader
{
public:
    KeywordLoader(const std::filesystem::path& sdkRoot,
                  const std::vector<std::filesystem::path>& pluginDirs);

    virtual ~KeywordLoader() = 0;

    virtual bool populateIndex(KeywordIndex* index) = 0;

protected:
    const std::filesystem::path sdkRoot_;
    const std::vector<std::filesystem::path> pluginDirs_;
};

}
