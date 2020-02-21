#pragma once

#include <string>
#include <vector>

namespace odbc {

class Keyword
{
public:
    std::string name;
    std::string helpFile;
    std::vector<std::vector<std::string>> overloads;
    bool hasReturnType = false;
};

}
