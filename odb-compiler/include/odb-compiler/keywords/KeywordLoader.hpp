#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace odb {
class DynamicLibrary;

namespace kw {
class KeywordIndex;

class ODBCOMPILER_PUBLIC_API KeywordLoader
{
public:
    KeywordLoader(const std::string& sdkRoot,
                  const std::vector<std::string>& pluginDirs={});

    virtual ~KeywordLoader() {}

    virtual bool populateIndex(KeywordIndex* index) = 0;
    virtual bool populateIndexFromLibrary(KeywordIndex* index, DynamicLibrary* library) = 0;

protected:
    const std::filesystem::path sdkRoot_;
    const std::vector<std::filesystem::path> pluginDirs_;
};

}
}
