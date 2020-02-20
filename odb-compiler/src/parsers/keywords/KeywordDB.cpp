#include "odbc/parsers/keywords/KeywordDB.hpp"
#include "odbc/parsers/keywords/Driver.hpp"
#include <cstdio>
#include <iostream>

namespace odbc {

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

    odbc::kw::Driver driver(this);
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
    return result.second;
}

// ----------------------------------------------------------------------------
const Keyword* KeywordDB::lookup(const std::string& keyword) const
{
    auto result = map_.find(keyword);
    if (result == map_.end())
        return nullptr;
    return &result->second;
}

// ----------------------------------------------------------------------------
void KeywordDB::printAll()
{
    for (const auto& kv : map_)
    {
        std::cout << kv.second.name << "=" << kv.second.helpFile << ": ";
        for (const auto& overload : kv.second.overloads)
        {
            std::cout << "[";
            auto it = overload.begin();
            if (it != overload.end())
                std::cout << *it++;
            while (it != overload.end())
                std::cout << ", " << *it++;
            std::cout << "]";
        }
        std::cout << std::endl;
    }
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordDB::keywordsAsList() const
{
    std::vector<std::string> list;
    list.reserve(map_.size());
    for (const auto& kv : map_)
        list.push_back(kv.first);
    return list;
}

}
