#include "odb-compiler/keywords/INIKeywordLoader.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/parsers/keywords/Driver.hpp"

namespace odb {

// ----------------------------------------------------------------------------
INIKeywordLoader::INIKeywordLoader(
                     const std::filesystem::path& sdkRoot,
                     const std::vector<std::filesystem::path>& pluginDirs) :
    KeywordLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool INIKeywordLoader::populateIndex(KeywordIndex* index)
{
    bool result;

    // XXX this is completely wrong, just made it compile for now
    FILE* fp = fopen(sdkRoot_.c_str(), "rb");
    if (fp == nullptr)
        return false;

    odb::kw::Driver driver(index);
    result = driver.parseStream(fp);

    fclose(fp);
    return result;
}

}
