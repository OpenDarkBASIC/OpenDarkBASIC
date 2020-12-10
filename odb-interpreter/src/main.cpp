#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-compiler/keywords/ODBKeywordLoader.hpp"
#include "odb-compiler/keywords/SDKType.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    odb::log::init();

    std::cout << "Loading plugin..." << std::endl;
    auto p = odb::DynamicLibrary::open("odb-sdk/plugins/test-plugin/test-plugin.so");
    if (p == nullptr)
        return 1;

    std::cout << "Reading keywords..." << std::endl;
    odb::KeywordIndex kwIndex;
    odb::ODBKeywordLoader loader("");
    loader.populateIndexFromLibrary(&kwIndex, p);

    for (const auto& name : kwIndex.keywordNamesAsList())
        std::cout << name << std::endl;

    return 0;
}
