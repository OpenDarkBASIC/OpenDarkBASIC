#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/parsers/keywords/Driver.hpp"
#include "odb-runtime/Plugin.hpp"
#include <cstdio>
#include <iostream>

namespace odb {

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
bool KeywordIndex::loadFromPlugin(const Plugin& plugin)
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
    return true;
}

// ----------------------------------------------------------------------------
bool KeywordIndex::addKeyword(Keyword keyword)
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
