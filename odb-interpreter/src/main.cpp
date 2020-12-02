#include "odb-compiler/parsers/keywords/Driver.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-runtime/Plugin.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Loading plugin..." << std::endl;
    auto p = odb::Plugin::open("odb-sdk/plugins/test-plugin/test-plugin.so");
    if (p == nullptr)
        return 1;

    std::cout << "Reading keywords..." << std::endl;
    odb::KeywordIndex kwIndex;
    kwIndex.loadFromPlugin(*p);

    for (const auto& name : kwIndex.keywordNamesAsList())
        std::cout << name << std::endl;

    return 0;
}
