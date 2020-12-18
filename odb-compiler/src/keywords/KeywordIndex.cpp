#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"
#include "odb-sdk/Str.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <unordered_map>

namespace odb {
namespace kw {

// ----------------------------------------------------------------------------
void KeywordIndex::addKeyword(Keyword* keyword)
{
    keywords_.push_back(keyword);
}

// ----------------------------------------------------------------------------
bool KeywordIndex::findConflicts() const
{
    std::unordered_map<std::string, std::vector<const Keyword*>> map_;

    for (const auto& kw : keywords_)
    {
        auto it = map_.insert({str::toLower(kw->dbSymbol()), {kw}});
        if (it.second)
            continue;

        // This dbSymbol already exists. Have to compare the new keyword with
        // all overloads of the existing symbol
        for (const auto& overload : it.first->second)
        {
            auto compare = [](const Keyword* a, const Keyword* b) -> bool {
                // Compare each argument type
                for (std::size_t i = 0; i != a->args().size() && i != b->args().size(); ++i)
                {
                    if (a->args()[i].type != b->args()[i].type)
                        return false;
                }
                if (a->args().size() != b->args().size())
                    return false;

                // Compare return type
                if (a->returnType() != b->returnType())
                    return false;

                return true;
            };

            if (compare(kw, overload))
            {
                std::string typeinfo;
                typeinfo.push_back(static_cast<char>(kw->returnType()));
                typeinfo.push_back('(');
                for (const auto& arg : kw->args())
                    typeinfo.push_back(static_cast<char>(arg.type));
                typeinfo.push_back(')');

                log::sdk(log::ERROR, "Keyword `%s %s` redefined in library `%s`", kw->dbSymbol().c_str(), typeinfo.c_str(), kw->library()->getFilename());
                log::sdk(log::NOTICE, "Keyword was first declared in library `%s`", overload->library()->getFilename());
                return true;
            }
        }
    }

    return false;
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Keyword>>& KeywordIndex::keywords() const
{
    return keywords_;
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordIndex::keywordNamesAsList() const
{
    std::vector<std::string> list;
    list.reserve(keywords_.size());
    for (const auto& kw : keywords_)
        list.push_back(kw->dbSymbol());
    return list;
}

// ----------------------------------------------------------------------------
std::vector<std::string> KeywordIndex::librariesAsList() const
{
    std::vector<std::string> list;
    list.reserve(keywords_.size());
    for (const auto& kw : keywords_)
        list.push_back(kw->library()->getFilename());
    return list;
}

}
}
