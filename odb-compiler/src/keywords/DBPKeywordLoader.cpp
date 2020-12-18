#include "odb-compiler/keywords/DBPKeywordLoader.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/FileSystem.hpp"
#include "odb-sdk/Str.hpp"
#include <unordered_set>
#include <set>

namespace fs = std::filesystem;

namespace odb {
namespace kw {

// ----------------------------------------------------------------------------
DBPKeywordLoader::DBPKeywordLoader(const std::string& sdkRoot,
                                   const std::vector<std::string>& pluginDirs) :
    KeywordLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool DBPKeywordLoader::populateIndex(KeywordIndex* index)
{
    std::unordered_set<std::string> pluginsToLoad;

    if (!fs::is_directory(sdkRoot_))
    {
        log::sdk(log::ERROR, "SDK root directory `%s` does not exist", sdkRoot_.c_str());
        return false;
    }

    fs::path sdkPluginsDir = sdkRoot_ / "plugins";
    if (!fs::is_directory(sdkPluginsDir))
    {
        log::sdk(log::WARNING, "`%s` does not exist. SDK Plugins will not be loaded", sdkPluginsDir.c_str());
    }
    else
    {
        for (const auto& p : fs::recursive_directory_iterator(sdkPluginsDir))
            if (fileIsDynamicLib(p.path()))
                pluginsToLoad.emplace(p.path().string());
    }

    for (const auto& path : pluginDirs_)
    {
        if (!fs::is_directory(path))
        {
            log::sdk(log::WARNING, "`%s` does not exist. Skipping.", path.c_str());
            continue;
        }

        for (const auto& p : fs::recursive_directory_iterator(sdkPluginsDir))
            if (fileIsDynamicLib(p.path()))
                pluginsToLoad.emplace(p.path().string());
    }

    for (const auto& path : pluginsToLoad)
    {
        Reference<DynamicLibrary> lib = DynamicLibrary::open(path.c_str());
        if (lib == nullptr)
            continue;

        if (!populateIndexFromLibrary(index, lib))
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool DBPKeywordLoader::populateIndexFromLibrary(KeywordIndex* index, DynamicLibrary* library)
{
    std::set<std::string> stringTable;

    // Load string table from library. We use a set here to deal with any duplicate entries in the library.
    int stringTableSize = library->getStringTableSize();
    for (int i = 0; i < stringTableSize; ++i)
    {
        stringTable.emplace(library->getStringTableEntryAt(i));
    }

    // Parse string table.
    for (const auto& stringTableEntry : stringTable)
    {
        std::vector<std::string> tokens;
        str::split(stringTableEntry, tokens, '%');

        if (tokens.size() < 2)
        {
            fprintf(stderr, "Invalid string table entry: %s\n", stringTableEntry.c_str());
            continue;
        }

        auto convertTypeChar = [](char type) -> Keyword::Type
        {
            return static_cast<Keyword::Type>(type);
        };

        // Extract keyword name and return type.
        auto& keywordName = tokens[0];
        auto& functionTypes = tokens[1];
        const auto& dllSymbol = tokens[2];
        Keyword::Type returnType = Keyword::Type::Void;
        std::vector<Keyword::Arg> args;

        // Extract return type.
        if (keywordName.back() == '[')
        {
            keywordName = keywordName.substr(0, keywordName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            functionTypes = functionTypes.substr(1);
        }
        std::transform(keywordName.begin(), keywordName.end(), keywordName.begin(), [](char c) { return std::tolower(c); });

        // Extract arguments.
        std::vector<std::string> argumentNames;
        if (tokens.size() > 3)
        {
            str::split(tokens[3], argumentNames, ',');
        }
        for (int typeIdx = 0; typeIdx < functionTypes.size(); ++typeIdx)
        {
            Keyword::Arg arg;
            arg.type = convertTypeChar(functionTypes[typeIdx]);
            if (arg.type == Keyword::Type::Void)
            {
                continue;
            }
            if (typeIdx < argumentNames.size())
            {
                arg.description = std::move(argumentNames[typeIdx]);
            }
            args.emplace_back(std::move(arg));
        }

        index->addKeyword(new Keyword(library, keywordName, dllSymbol, returnType, args));
    }

    return true;
}

}
}
