#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/KeywordLoader.hpp"

namespace odb {
namespace kw {

class DBPKeywordLoader : public KeywordLoader
{
public:
    DBPKeywordLoader(const std::string& sdkRoot,
                     const std::vector<std::string>& pluginDirs);

    bool populateIndex(KeywordIndex* index) override;
    bool populateIndexFromLibrary(KeywordIndex* index, DynamicLibrary* library) override;
};

}
}
