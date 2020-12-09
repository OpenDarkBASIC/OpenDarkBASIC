#include "odb-compiler/keywords/KeywordIndex.hpp"

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
bool KeywordIndex::loadFromPlugin(const Plugin& plugin, SDKType sdkType)
{

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
