#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/KeywordLoader.hpp"

namespace odb {

class DBPKeywordLoader : public KeywordLoader
{
public:
    DBPKeywordLoader(const std::filesystem::path& sdkRoot,
                     const std::vector<std::filesystem::path>& pluginDirs);

    bool populateIndex(KeywordIndex* index) override;
};

}
