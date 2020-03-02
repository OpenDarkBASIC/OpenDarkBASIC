#include "odbc/parsers/keywords/KeywordDB.hpp"
#include "odbc/parsers/keywords/Driver.hpp"
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/util/Plugin.hpp"

using namespace odbc;

int main(int argc, char** argv)
{
    Plugin* p = Plugin::load("odb-sdk/test-plugin/test-plugin.so");
    if (p == nullptr)
        return 1;
    KeywordDB db;
    p->loadKeywords(&db);
    delete p;
    return 0;
}
