#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/Keyword.hpp"
#include <unordered_map>

namespace odbc {

class KeywordDB
{
public:
    ODBC_PUBLIC_API bool loadFromDirectory(const std::string& dir);
    ODBC_PUBLIC_API bool loadFromFile(const std::string& fileName);
    ODBC_PUBLIC_API bool exists(const std::string& keyword);

    ODBC_PUBLIC_API bool addKeyword(Keyword keyword);
    ODBC_PUBLIC_API const Keyword* lookup(const std::string& keyword) const;

    ODBC_PUBLIC_API std::vector<Keyword> keywordsAsList() const;
    ODBC_PUBLIC_API std::vector<std::string> keywordNamesAsList() const;

private:
    std::unordered_map<std::string, Keyword> map_;
};

}
