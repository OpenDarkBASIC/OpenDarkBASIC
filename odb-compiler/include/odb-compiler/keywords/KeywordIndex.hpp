#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include "odb-sdk/Reference.hpp"
#include <unordered_map>
#include <vector>

namespace odb {

class Keyword;

class KeywordIndex
{
public:
    ODBCOMPILER_PUBLIC_API bool addKeyword(Keyword* keyword);
    ODBCOMPILER_PUBLIC_API Keyword* lookup(const std::string& keyword);

    ODBCOMPILER_PUBLIC_API int keywordCount() const;
    ODBCOMPILER_PUBLIC_API std::vector<Keyword*> keywordsAsList() const;
    ODBCOMPILER_PUBLIC_API std::vector<std::string> keywordNamesAsList() const;

    ODBCOMPILER_PUBLIC_API std::vector<std::string> pluginsAsList() const;

private:
    std::unordered_map<std::string, Reference<Keyword>> map_;
};

}
