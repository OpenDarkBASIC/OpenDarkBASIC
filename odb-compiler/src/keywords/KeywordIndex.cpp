#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/parsers/keywords/Driver.hpp"
#include "odb-sdk/Plugin.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>

namespace odb {

namespace {

template <class Container>
void split(const std::string &str, Container &cont,
           char delim = ' ')
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos)
    {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

}

// ----------------------------------------------------------------------------
bool KeywordIndex::loadFromINIFile(const std::string& fileName)
{
    bool result;

    FILE* fp = fopen(fileName.c_str(), "rb");
    if (fp == nullptr)
        return false;

    odb::kw::Driver driver(this);
    result = driver.parseStream(fp);

    fclose(fp);
    return result;
}

// ----------------------------------------------------------------------------
bool KeywordIndex::loadFromPlugin(const Plugin& plugin, SDKType sdkType)
{
    if (sdkType == SDKType::ODB)
    {
        const char* keyword;
        const char* typeinfo;
        const char* helpfile;

        auto lookupString = [&plugin](std::string sym) -> const char* {
            const char** addr = reinterpret_cast<const char**>(
                plugin.lookupSymbolAddress(sym.c_str()));
            return addr ? *addr : nullptr;
        };

        for (int i = 0; i != plugin.getSymbolCount(); ++i)
        {
            std::string sym = plugin.getSymbolAt(i);

            if (!(keyword = lookupString(sym + "_keyword")))
                continue;
            if (!(typeinfo = lookupString(sym + "_typeinfo")))
                continue;
            helpfile = lookupString(sym + "_helpfile");  // optional symbol

            addKeyword({keyword, plugin.getName(), helpfile, {}, std::nullopt});
        }
    }
    else if (sdkType == SDKType::DarkBASIC)
    {
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
    }
    return true;
}

// ----------------------------------------------------------------------------
bool KeywordIndex::addKeyword(const Keyword& keyword)
{
    auto result = map_.insert({keyword.name, keyword});
    plugins_.insert(keyword.plugin);
    return result.second;
}

// ----------------------------------------------------------------------------
Keyword* KeywordIndex::lookup(const std::string& keyword)
{
    auto result = map_.find(keyword);
    if (result == map_.end())
        return nullptr;
    return &result->second;
}

// ----------------------------------------------------------------------------
const Keyword* KeywordIndex::lookup(const std::string& keyword) const
{
    const auto result = map_.find(keyword);
    if (result == map_.end())
        return nullptr;
    return &result->second;
}

// ----------------------------------------------------------------------------
int KeywordIndex::keywordCount() const
{
    return map_.size();
}

// ----------------------------------------------------------------------------
std::vector<Keyword> KeywordIndex::keywordsAsList() const
{
    std::vector<Keyword> list;
    list.reserve(map_.size());
    for (const auto& kv : map_)
        list.push_back(kv.second);
    return list;
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordIndex::keywordNamesAsList() const
{
    std::vector<std::string> list;
    list.reserve(map_.size());
    for (const auto& kv : map_)
        list.push_back(kv.first);
    return list;
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordIndex::pluginsAsList() const
{
    std::vector<std::string> list;
    list.reserve(plugins_.size());
    for (const auto& plugin : plugins_)
        list.push_back(plugin);
    return list;
}

}
