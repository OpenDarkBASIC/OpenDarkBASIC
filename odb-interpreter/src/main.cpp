#include "odb-compiler/parsers/keywords/Driver.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/keywords/KeywordDB.hpp"
#include "odb-sdk/runtime/Plugin.hpp"

int main(int argc, char** argv)
{
    odb::Plugin* p = odb::Plugin::load("odb-sdk/test-plugin/test-plugin.so");
    if (p == nullptr)
        return 1;
    odb::KeywordDB db;
    p->loadKeywords(&db);
    delete p;
    return 0;
}
