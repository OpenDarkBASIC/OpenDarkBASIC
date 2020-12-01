#include "odb-compiler/keywords/KeywordDB.hpp"
#include "odb-compiler/parsers/keywords/Driver.hpp"
#include <cstdio>
#include <iostream>

namespace odb {

// ----------------------------------------------------------------------------
bool KeywordDB::loadFromDirectory(const std::string& dir)
{
    return false;
}

// ----------------------------------------------------------------------------
bool KeywordDB::loadFromFile(const std::string& fileName)
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
bool KeywordDB::exists(const std::string& keyword)
{
    return map_.find(keyword) != map_.end();
}

// ----------------------------------------------------------------------------
bool KeywordDB::addKeyword(Keyword keyword)
{
    auto result = map_.insert({keyword.name, keyword});
    plugins_.insert(keyword.plugin);
    return result.second;
}

// ----------------------------------------------------------------------------
Keyword* KeywordDB::lookup(const std::string& keyword)
{
    auto result = map_.find(keyword);
    if (result == map_.end())
        return nullptr;
    return &result->second;
}

// ----------------------------------------------------------------------------
const Keyword* KeywordDB::lookup(const std::string& keyword) const
{
    const auto result = map_.find(keyword);
    if (result == map_.end())
        return nullptr;
    return &result->second;
}

// ----------------------------------------------------------------------------
std::vector<Keyword> KeywordDB::keywordsAsList() const
{
    std::vector<Keyword> list;
    list.reserve(map_.size());
    for (const auto& kv : map_)
        list.push_back(kv.second);
    return list;
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordDB::keywordNamesAsList() const
{
    std::vector<std::string> list;
    list.reserve(map_.size());
    for (const auto& kv : map_)
        list.push_back(kv.first);
    return list;
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordDB::pluginsAsList() const
{
    std::vector<std::string> list;
    list.reserve(plugins_.size());
    for (const auto& plugin : plugins_)
        list.push_back(plugin);
    return list;
}

}
