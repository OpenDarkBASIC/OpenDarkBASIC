#include <gmock/gmock.h>
#include "odb-util/DynamicLibrary.hpp"

#define NAME DynamicLibrary
#if defined(ODBUTIL_PLATFORM_LINUX)
#   define TEST_PLUGIN "odb-util/plugins/test-plugin.so"
#elif defined(ODBUTIL_PLATFORM_WINDOWS)
#   define TEST_PLUGIN "odb-util/plugins/test-plugin.dll"
#endif

using namespace testing;

TEST(NAME, load_and_unload)
{
    odb::DynamicLibrary lib = odb::DynamicLibrary::open(TEST_PLUGIN);
    ASSERT_THAT(lib, IsTrue());
}

TEST(NAME, iterate_symbols)
{
    static const char* expectedSymbols[] = {
        "test_command",
        "test_command_helpfile",
        "test_command_name",
        "test_command_typeinfo",
        nullptr
    };

    odb::DynamicLibrary lib = odb::DynamicLibrary::open(TEST_PLUGIN);
    ASSERT_THAT(lib, IsTrue());

    std::set<std::string> testCommandSymbols;
    lib.forEachSymbol([&testCommandSymbols](const char* sym) {
        if (std::string(sym).rfind("test_command", 0) != std::string::npos)
            testCommandSymbols.insert(sym);
        return odb::DynamicLibrary::CONTINUE;
    });

    auto setContains = [&testCommandSymbols](const char* test) -> bool {
        return testCommandSymbols.find(test) != testCommandSymbols.end();
    };

    for (const char** expected = expectedSymbols; *expected; ++expected) {
        EXPECT_THAT(setContains(*expected), IsTrue());
    }
}

TEST(NAME, lookup_function)
{
    odb::DynamicLibrary lib = odb::DynamicLibrary::open(TEST_PLUGIN);
    ASSERT_THAT(lib, IsTrue());

    EXPECT_THAT(lib.lookupFunction<void>("test_command"), NotNull());
    EXPECT_THAT(lib.lookupFunction<void>("doesnt_exist"), IsNull());
}

TEST(NAME, lookup_symbol)
{
    odb::DynamicLibrary lib = odb::DynamicLibrary::open(TEST_PLUGIN);
    ASSERT_THAT(lib, IsTrue());

    EXPECT_THAT(lib.lookupSymbol<void*>("test_command_name"), NotNull());
    EXPECT_THAT(lib.lookupSymbol<void*>("doesnt_exist"), IsNull());
}

TEST(NAME, lookup_string_symbol)
{
    odb::DynamicLibrary lib = odb::DynamicLibrary::open(TEST_PLUGIN);
    ASSERT_THAT(lib, IsTrue());

    EXPECT_THAT(lib.lookupStringSymbol("test_command_name"), StrEq("test command"));
    EXPECT_THAT(lib.lookupStringSymbol("doesnt_exist"), StrEq(""));
}

#if defined(ODBUTIL_PLATFORM_WINDOWS)
TEST(NAME, iterate_string_resource_table)
{
    static const char* expectedStrings[] = {
        "first string",
        "fourth string",
        "second string",
        "fifth string",
        "third string",
        "sixth string"
    };

    odb::DynamicLibrary lib = odb::DynamicLibrary::open(TEST_PLUGIN);
    ASSERT_THAT(lib, IsTrue());

    std::set<std::string> stringsFound;
    lib.forEachString([&stringsFound](const char* str) {
        stringsFound.insert(str);
            std::cout << str << std::endl;
        return odb::DynamicLibrary::CONTINUE;
    });

    auto setContains = [&stringsFound](const char* test) -> bool {
        return stringsFound.find(test) != stringsFound.end();
    };

    for (const char** expected = expectedStrings; *expected; ++expected) {
        EXPECT_THAT(setContains(*expected), IsTrue());
    }
}
#endif
