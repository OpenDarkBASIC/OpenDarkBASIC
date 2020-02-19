#include "odbc/parsers/keywords/KeywordsDB.hpp"
#include <iostream>

namespace odbc {

// ----------------------------------------------------------------------------
void KeywordDB::appendFromFile(const std::string& fileName)
{
}

// ----------------------------------------------------------------------------
bool KeywordDB::exists(const std::string& keyword)
{
    return map_.find(keyword) != map_.end();
}

// ----------------------------------------------------------------------------
bool KeywordDB::addKeyword(Keyword keyword)
{
    std::cout << keyword.name << "=" << keyword.helpFile << ": ";
    if (keyword.hasReturnType)
        std::cout << "(";
    for (const auto& overload : keyword.overloads)
    {
        std::cout << "[";
        auto it = overload.begin();
        if (it != overload.end())
            std::cout << *it++;
        while (it != overload.end())
            std::cout << ", " << *it++;
        std::cout << "]";
    }
    if (keyword.hasReturnType)
        std::cout << ")";
    std::cout << std::endl;

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

}
