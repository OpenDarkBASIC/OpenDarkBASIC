#include <gmock/gmock.h>
#include "odb-sdk/DynamicLibrary.hpp"

#define NAME DynamicLibrary

using namespace testing;

TEST(NAME, load_and_unload)
{
    odb::DynamicLibrary lib = odb::DynamicLibrary::open("odb-sdk/plugins/test-plugin.dll");
    ASSERT_THAT(lib, IsTrue());
    for (auto sym : lib.symbols())
        std::cout << sym << std::endl;
}
