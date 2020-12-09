#include "odb-compiler/keywords/DBPKeywordLoader.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"

namespace odb {

// ----------------------------------------------------------------------------
DBPKeywordLoader::DBPKeywordLoader(const std::filesystem::path& sdkRoot,
                                   const std::vector<std::filesystem::path>& pluginDirs) :
    KeywordLoader(sdkRoot, pluginDirs)
{
}

// ----------------------------------------------------------------------------
bool DBPKeywordLoader::populateIndex(KeywordIndex* index)
{
#if 0
    int stringTableSize = plugin.getStringTableSize();
    for (int i = 0; i < stringTableSize; ++i)
    {
        std::vector<std::string> tokens;
        std::string stringTableEntry = plugin.getStringTableEntryAt(i);
        split(stringTableEntry, tokens, '%');

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
        std::optional<Keyword::Type> returnType;

        // Extract return type.
        if (keywordName.back() == '[')
        {
            keywordName = keywordName.substr(0, keywordName.size() - 1);
            returnType = convertTypeChar(tokens[1][0]);
            functionTypes = functionTypes.substr(1);
        }
        std::transform(keywordName.begin(), keywordName.end(), keywordName.begin(), [](char c) { return std::tolower(c); });

        // Create overload.
        Keyword::Overload overload;
        overload.symbolName = dllSymbol;

        std::vector<std::string> argumentNames;
        if (tokens.size() > 3)
        {
            split(tokens[3], argumentNames, ',');
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
            overload.arglist.emplace_back(std::move(arg));
        }

        // Add to database, or merge with existing keyword if it exists already.
        Keyword* existingKeyword = lookup(keywordName);
        if (existingKeyword)
        {
            existingKeyword->overloads.emplace_back(std::move(overload));
        }
        else
        {
            addKeyword({std::move(keywordName), plugin.getName(), "", {std::move(overload)}, returnType});
        }
    }

    return true;
#endif

    return false;
}
