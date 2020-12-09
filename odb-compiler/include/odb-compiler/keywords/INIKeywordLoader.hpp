#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/KeywordLoader.hpp"

namespace odb {

class INIKeywordLoader : public KeywordLoader
{
public:
    INIKeywordLoader(const std::filesystem::path& sdkRoot,
                     const std::vector<std::filesystem::path>& pluginDirs);

    bool populateIndex(KeywordIndex* index) override;
};

}
