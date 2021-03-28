#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/commands/CommandIndex.hpp"
#include "odb-compiler/commands/ODBCommandLoader.hpp"
#include "odb-compiler/commands/SDKType.hpp"
#include "odb-compiler/parsers/DynamicLibData.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-sdk/DynamicLibrary.hpp"
#include "odb-sdk/Log.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "Loading plugin..." << std::endl;
    auto p = odb::DynamicLibData::open("odb-sdk/plugins/test-plugin.so");
    if (p == nullptr)
        return 1;

    std::cout << "Reading commands..." << std::endl;
    odb::cmd::CommandIndex cmdIndex;
    odb::cmd::ODBCommandLoader loader("");
    loader.populateIndexFromLibrary(&cmdIndex, p);

    for (const auto& name : cmdIndex.commandNamesAsList())
        std::cout << name << std::endl;

    return 0;
}
