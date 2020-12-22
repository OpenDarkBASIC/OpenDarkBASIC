#pragma once

#include "odb-compiler/parsers/db/Parser.y.hpp"
#include <string>

namespace odb {
namespace db {

class KeywordToken
{
public:
    struct Result { const char* name; dbtokentype token; };

    static const Result* lookup(const std::string& keyword);
};

}
}
