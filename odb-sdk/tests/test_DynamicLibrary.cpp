#include <gmock/gmock.h>
#include "odb-sdk/DynamicLibrary.hpp"

#define NAME DynamicLibrary

using namespace testing;

TEST(NAME, load_and_unload)
{
    odb::DynamicLibrary lib = odb::DynamicLibrary::open("odb-sdk/plugins/test-plugin.dll");
    ASSERT_THAT(lib, IsTrue());

//#if defined(ODB_PLATFORM_WINDOWS)
    static const char* expectedStrings[] = {
        "first string",
        "fourth string",
        "second string",
        "fifth string",
        "third string",
        "sixth string"
    };
//#endif

    static const char* expectedSymbols[] = {
        "test_command",
        "test_command_helpfile",
        "test_command_name",
        "test_command_typeinfo"
    };

    int i = 0;
    lib.forEachSymbol([&i](const char* sym) -> bool {
        EXPECT_THAT();
        return true;
    });

    lib.forEachString([](const char* str) -> bool {
        std::cout << str << std::endl;
        return true;
    });
}
