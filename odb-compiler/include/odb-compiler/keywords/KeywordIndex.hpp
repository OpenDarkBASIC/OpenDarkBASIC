#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include <unordered_map>
#include <unordered_set>

namespace odb {

class Plugin;

class KeywordIndex
{
public:
    ODBCOMPILER_PUBLIC_API bool loadFromINIFile(const std::string& fileName);
    ODBCOMPILER_PUBLIC_API bool loadFromPlugin(const Plugin& plugin, SDKType sdkType);

    ODBCOMPILER_PUBLIC_API bool addKeyword(const Keyword& keyword);
    ODBCOMPILER_PUBLIC_API Keyword* lookup(const std::string& keyword);
    ODBCOMPILER_PUBLIC_API const Keyword* lookup(const std::string& keyword) const;

    ODBCOMPILER_PUBLIC_API int keywordCount() const;
    ODBCOMPILER_PUBLIC_API std::vector<Keyword> keywordsAsList() const;
    ODBCOMPILER_PUBLIC_API std::vector<std::string> keywordNamesAsList() const;

    ODBCOMPILER_PUBLIC_API std::vector<std::string> pluginsAsList() const;

private:
    std::unordered_map<std::string, Keyword> map_;
    std::unordered_set<std::string> plugins_;
};

}
